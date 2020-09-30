//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "..\.\pbc\pbc_dataConvert\pbc_dataConvert.h"
#include ".\mde_storage.h"
#include ".\depend\bsp_storage.h"
//-----------------------------------------------------------------------------
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	  |                      |-- serial number(anticode)
//	  |                      |-- upgrade flag(anticode)
//	  |                      |-- checksum(anticode)
//	  |                      |-- execution code size(anticode)
//	  |                      |-- serial number
//	  |                      |-- upgrade flag
//	  |                      |              | 
//	  |                      |              |-- 0x519AE493--complete  0x8D731A75--updating
//	  |                      |-- checksum
//	  |                      ‘-- execution code size
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define upgrade_complete   0x519AE493
#define upgrade_updating   0x8D731A75
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//反码校验处理
//-----------------------------------------------------------------------------
static sdt_bool check_notcode_is_ok(sdt_int8u* in_pData,sdt_int8u size)
{
    sdt_int8u i;
    sdt_int8u check_txt = 0;
    
    for(i = 0;i < size;i ++)
    {
        check_txt += in_pData[i];
        if(i&0x01)
        {
            check_txt += 1;
        }
    }
    if(check_txt)
    {
        return(sdt_false);
    }
    return(sdt_true);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CheckSumWordEven          0x39ea2e76
#define CheckSumWordOdd           0x82b453c3
//-----------------------------------------------------------------------------
static sdt_int32u MakeOneCheckText(sdt_int32u Data,sdt_int32u Count)
{
    if(Count&0x00000001)
    {
        Data=Data^CheckSumWordOdd;
    }
    else
    {
        Data=Data^CheckSumWordEven;
    }
    return(Data);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//升级区数据校验
//-----------------------------------------------------------------------------
static sdt_bool upgrade_checksum_is_ok(sdt_int32u checksum,sdt_int32u codesize)
{
    sdt_int32u offset_addr = 0;
    sdt_int8u rd_buff[4];
    sdt_int32u make_checksum = 0;
    sdt_int32u rd_data;
    sdt_int32u count = 0;
    
    codesize = codesize/4;  //折算成32bits的大小
    while(codesize)
    {
        bsp_read_4bytes_user_upgrade(offset_addr,&rd_buff[0]);
        rd_data = pbc_arrayToInt32u_bigEndian(&rd_buff[0]);
        make_checksum += MakeOneCheckText(rd_data,count);
        offset_addr += 4;
        codesize --;
        count++;
    }
    if(checksum == make_checksum)
    {
        return(sdt_true);
    }
    return(sdt_false);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//运行区数据校验
//-----------------------------------------------------------------------------
static sdt_bool app_checksum_is_ok(sdt_int32u checksum,sdt_int32u codesize)
{
    sdt_int32u offset_addr = 0;
    sdt_int8u rd_buff[4];
    sdt_int32u make_checksum = 0;
    sdt_int32u rd_data;
    sdt_int32u count = 0;
    
    codesize = codesize/4;  //折算成32bits的大小
    while(codesize)
    {
        bsp_read_4bytes_user_app(offset_addr,&rd_buff[0]);
        rd_data = pbc_arrayToInt32u_bigEndian(&rd_buff[0]);
        make_checksum += MakeOneCheckText(rd_data,count);
        offset_addr += 4;
        codesize --;
        count++;
    }
    if(checksum == make_checksum)
    {
        return(sdt_true);
    }
    return(sdt_false);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void mde_need_upgrade(void)
{
    sdt_int8u rd_inf[32];
    bsp_read_information_user_upgrade(&rd_inf[0]);
    if(check_notcode_is_ok(&rd_inf[0],sizeof(rd_inf)))
    {
        sdt_bool need_upgrade = sdt_false;
        sdt_int32u update_srm;
        sdt_int32u update_flag;
        sdt_int32u update_checksum;
        sdt_int32u update_codesize;
        
        update_srm = pbc_arrayToInt32u_bigEndian(&rd_inf[16]);        //序号,升级区每升级一次，序号加1
        update_flag = pbc_arrayToInt32u_bigEndian(&rd_inf[16+4]);     //标记
        update_checksum = pbc_arrayToInt32u_bigEndian(&rd_inf[16+8]); //校验文
        update_codesize = pbc_arrayToInt32u_bigEndian(&rd_inf[16+12]);//代码字节数
        
        if(upgrade_complete == update_flag)
        {
            if(upgrade_checksum_is_ok(update_checksum,update_codesize))
            {
                bsp_read_information_user_app(&rd_inf[0]);
                if(check_notcode_is_ok(&rd_inf[0],sizeof(rd_inf)))
                {
                    sdt_int32u app_srm;
                    sdt_int32u app_checksum;
                    sdt_int32u app_codesize;
                    
                    app_srm = pbc_arrayToInt32u_bigEndian(&rd_inf[16]);
                    app_checksum = pbc_arrayToInt32u_bigEndian(&rd_inf[16+8]);
                    app_codesize = pbc_arrayToInt32u_bigEndian(&rd_inf[16+12]);
                    if(app_checksum_is_ok(app_checksum,app_codesize))
                    {
                        if(update_srm != app_srm)
                        {
                            need_upgrade = sdt_true;
                        }                        
                    }
                    else
                    {
                        need_upgrade = sdt_true;
                    }
                }
                else
                {
                    need_upgrade = sdt_true;
                }                
            }
             
            if(need_upgrade) //need upgrade
            {//transfet data
                bsp_transfer_user_upgrade_to_app(update_codesize);
            }
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void mde_jump_to_user_app(void)
{
    bsp_jump_to_user_app();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_bool mde_user_app_is_legal(void)
{
    sdt_bool legal = sdt_false;
    sdt_int8u rd_inf[32];
   
    bsp_read_information_user_app(&rd_inf[0]);
    if(check_notcode_is_ok(&rd_inf[0],sizeof(rd_inf)))
    {
        sdt_int32u app_checksum;
        sdt_int32u app_codesize;

        app_checksum = pbc_arrayToInt32u_bigEndian(&rd_inf[16+8]);
        app_codesize = pbc_arrayToInt32u_bigEndian(&rd_inf[16+12]);
        if(app_checksum_is_ok(app_checksum,app_codesize))
        {
            legal = sdt_true;
        }
    }
    return(legal);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++