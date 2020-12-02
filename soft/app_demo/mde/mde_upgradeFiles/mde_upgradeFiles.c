//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\depend\bsp_upgradeFiles.h"
#include ".\mde_upgradeFiles.h"
#include "..\..\pbc\pbc_sysTick\pbc_sysTick.h"
#include "..\..\pbc\pbc_dataConvert\pbc_dataConvert.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//fireware update task
//-------------------------------------------------------------------------------------------------
typedef enum
{
    fwUpdate_status_waitFilesMap       = 0x00,
    fwUpdate_status_receiveFiles       = 0x01,
    fwUpdate_status_complete           = 0x02,
    fwUpdate_status_restart            = 0x03,
}fwUpdate_status_def;
//-------------------------------------------------------------------------------------------------
static fwUpdate_status_def fwUpdate_status = fwUpdate_status_waitFilesMap;
static sdt_int16u fwBlock_number;
macro_createTimer(timer_reset,timerType_millisecond,0);
macro_createTimer(timer_updateTimeout,timerType_second,0);
//-------------------------------------------------------------------------------------------------
void mde_upgrade_files_task(void)
{
    pbc_timerClockRun_task(&timer_reset);
    pbc_timerClockRun_task(&timer_updateTimeout);
    switch(fwUpdate_status)
    {
        case fwUpdate_status_waitFilesMap:
        {
            break;
        }
        case fwUpdate_status_receiveFiles:
        {
            break;
        }
        case fwUpdate_status_complete:
        {
            pbc_reload_timerClock(&timer_reset,1000);
            fwUpdate_status = fwUpdate_status_restart;
            break;
        }
        case fwUpdate_status_restart:
        {
            if(pbc_pull_timerIsCompleted(&timer_reset))
            {
                while(1)//; //wait watchdog timeout
                {
                    //mde_watchdog_reload();
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CheckSumWordEven          0x39ea2e76
#define CheckSumWordOdd           0x82b453c3
//----------------------------------------------------------------------------
sdt_int32u MakeOneCheckText(sdt_int32u Data,sdt_int32u Count)
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
//--------------------------------------------------------------------------------
//生成校验和
//入口:8bits的数据指针，数据长度
//出口:32bit的校验和
//--------------------------------------------------------------------------------
sdt_int32u MakeCheckSumText(sdt_int8u* pData,sdt_int32u Length)
{
    sdt_int32u iWords,This_ids;
    sdt_int32u Index;
    sdt_int32u MakeSum,ReadData;
    iWords = (Length)/4;
    This_ids = 0;
    MakeSum = 0;
    Index = 0;
    while(iWords)
    {
        ReadData = (sdt_int32u)pData[Index]&0x000000ff;
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+1]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+2]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((sdt_int32u)pData[Index+3]&0x000000ff);
        MakeSum+=MakeOneCheckText(ReadData,This_ids);
        Index = Index+4;
        iWords--;
        This_ids++;
    }
    return(MakeSum);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//#define KEYVAL  0x89,0x8E,0x3C,0x72,0xA7,0x19,0xF4,0x5D
//-------------------------------------------------------------------------------------------------
//名称: 文件解码
//功能: 传入密文，传出明文
//入口: 
//        _Mix_binf   文件入口指针                    <<---->>
//        _In_Length  文件长度,必须是128的倍数        <<----
//出口: 
//-------------------------------------------------------------------------------------------------
static void Decrypt(sdt_int8u* _Mix_binf,sdt_int8u _In_Length)
{
    sdt_int8u Key[8]={(sdt_int8u)0x89,(sdt_int8u)0x8E,(sdt_int8u)0x3C,(sdt_int8u)0x72,(sdt_int8u)0xA7,(sdt_int8u)0x19,(sdt_int8u)0xF4,(sdt_int8u)0x5D,};
    sdt_int8u i;
    sdt_int8u Length,BaseIndex;

    if(_In_Length < 128)
    {
        return;
    }
    
    Length = _In_Length;
    BaseIndex = 0;
    while(0!=Length)
    {
        Key[0] = (sdt_int8u)0x89;
        Key[1] = (sdt_int8u)0x8E;
        Key[2] = (sdt_int8u)0x3C;
        Key[3] = (sdt_int8u)0x72;
        Key[4] = (sdt_int8u)0xA7;
        Key[5] = (sdt_int8u)0x19;
        Key[6] = (sdt_int8u)0xF4;
        Key[7] = (sdt_int8u)0x5D;

        for(i=0 ;i<16 ;i++)
        {
            _Mix_binf[0+i*8+BaseIndex] = _Mix_binf[0+i*8+BaseIndex] ^ Key[0];
            _Mix_binf[1+i*8+BaseIndex] = _Mix_binf[1+i*8+BaseIndex] ^ Key[1];
            _Mix_binf[2+i*8+BaseIndex] = _Mix_binf[2+i*8+BaseIndex] ^ Key[2];
            _Mix_binf[3+i*8+BaseIndex] = _Mix_binf[3+i*8+BaseIndex] ^ Key[3];
            _Mix_binf[4+i*8+BaseIndex] = _Mix_binf[4+i*8+BaseIndex] ^ Key[4];
            _Mix_binf[5+i*8+BaseIndex] = _Mix_binf[5+i*8+BaseIndex] ^ Key[5];
            _Mix_binf[6+i*8+BaseIndex] = _Mix_binf[6+i*8+BaseIndex] ^ Key[6];
            _Mix_binf[7+i*8+BaseIndex] = _Mix_binf[7+i*8+BaseIndex] ^ Key[7];

            char lastkeybit;
            if(Key[0] & 0x80)
            {
                lastkeybit = 0x01;
            }
            else
            {
                lastkeybit = 0x00;
            }
            unsigned char ics;
            for(ics=0;ics<7;ics++)
            {
                Key[ics] = Key[ics]<<1;
                if(Key[ics+1] & 0x80)
                {
                    Key[ics] |= 0x01;
                }
            }
            Key[ics] = Key[ics]<<1;
            Key[ics] |= lastkeybit;
        }
        if(Length < 128)
        {
            break;
        }
        Length = Length-128;
        BaseIndex += 128;
    }
}
//--------------------------------------------------------------------------------------------------
static sdt_int32u makeData32u_bigEndian(sdt_int8u* in_data)
{
    sdt_int32u the_data;
    
    the_data = in_data[0];
    the_data = the_data<<8;
    the_data |= in_data[1];
    the_data = the_data<<8;
    the_data |= in_data[2];
    the_data = the_data<<8;
    the_data |= in_data[3];
    return(the_data);
}
//-------------------------------------------------------------------------------------------------
#define upgrade_complete   0x519AE493
#define upgrade_updating   0x8D731A75
//-------------------------------------------------------------------------------------------------
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//推入一个file map的报文
//-------------------------------------------------------------------------------------------------
sdt_int8u mde_push_fileMap(sdt_int8u* in_pBuff,sdt_bool in_resume)
{
    sdt_int8u error = BGEUERR_NoneError;
    sdt_int8u buff_block[128];
    sdt_int8u i;
    
    for(i = 0; i < 128; i++)
    {
        buff_block[i] = in_pBuff[i];
    }
    
    sdt_int32u checkSum;
    
    checkSum = makeData32u_bigEndian(&buff_block[124]);
    if(checkSum == MakeCheckSumText(&buff_block[0],124)) //信息块(32bytes)
    {

        sdt_int8u rd_inf[32];
        bsp_read_information_user_app(&rd_inf[0]); //读取当前信息块内容

        sdt_int32u fileMap_ver;
        sdt_int32u update_srm;
        sdt_int32u update_flag;
        sdt_int32u update_checksum;
        sdt_int32u update_codesize;
        
        update_srm = pbc_arrayToInt32u_bigEndian(&rd_inf[16]);        //序号,app区
        update_srm = update_srm + 1;
        fileMap_ver = pbc_arrayToInt32u_bigEndian(&buff_block[120]);  //version
        update_checksum = pbc_arrayToInt32u_bigEndian(&buff_block[108]);//校验文
        update_codesize = pbc_arrayToInt32u_bigEndian(&buff_block[112]);//代码字节数
        update_flag = upgrade_updating;    //标记

        if(0x00000001 == fileMap_ver)
        {
            sdt_int8u wr_inf[32];
            pbc_int32uToArray_bigEndian(~update_srm,&wr_inf[0]);
            pbc_int32uToArray_bigEndian(~update_flag,&wr_inf[4]);
            pbc_int32uToArray_bigEndian(~update_checksum,&wr_inf[8]);
            pbc_int32uToArray_bigEndian(~update_codesize,&wr_inf[12]);
            pbc_int32uToArray_bigEndian(update_srm,&wr_inf[16]);
            pbc_int32uToArray_bigEndian(update_flag,&wr_inf[20]);
            pbc_int32uToArray_bigEndian(update_checksum,&wr_inf[24]);
            pbc_int32uToArray_bigEndian(update_codesize,&wr_inf[28]);
            bsp_write_information_user_upgrade(&wr_inf[0]);       //写入信息到upgrade区
        }
        else
        {
            error = BGEUERR_Version;
        }
        
    }
    else
    {
        error = BGEUERR_FileMapCheckSum;
    }
    if(BGEUERR_NoneError == error)
    {
        if(in_resume)
        {
        }
        else
        {
            fwBlock_number = 0;
        }
        fwUpdate_status = fwUpdate_status_receiveFiles;        
    }

    return(error);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-------------------------------------------------------------------------------------------------
sdt_int8u mde_push_files_one_block(sdt_int8u in_block_num,sdt_int8u* in_pBuff)
{
    sdt_int8u buff_block[128];
    sdt_int8u i;
    sdt_int8u error = BGEUERR_NoneError;
    
    for(i = 0; i < 128; i++)
    {
        buff_block[i] = in_pBuff[i];
    }
    
    switch(fwUpdate_status)
    {
        case fwUpdate_status_receiveFiles:
        {
            if(in_block_num != (fwBlock_number+1))
            {
                error = BGEUERR_BlockNumber;
            }
            else
            {
                Decrypt(&buff_block[0],128);
                bsp_write_block_user_upgrade(fwBlock_number, &buff_block[0]);//写入一个块的数据到upgrade区

                fwBlock_number ++;
                
                sdt_int8u rd_inf[32];
                bsp_read_information_user_upgrade(&rd_inf[0]); //读取信息块内容

                //sdt_int32u fileMap_ver;
                //sdt_int32u update_srm;
                sdt_int32u update_flag;
                //sdt_int32u update_checksum;
                sdt_int32u update_codesize;

                update_codesize = pbc_arrayToInt32u_bigEndian(&rd_inf[28]);  
                if(fwBlock_number < (update_codesize / 128))
                {
                }
                else
                {
                    update_flag = upgrade_complete;
                    //pbc_int32uToArray_bigEndian(~update_srm,&rd_inf[0]);
                    pbc_int32uToArray_bigEndian(~update_flag,&rd_inf[4]);
                    //pbc_int32uToArray_bigEndian(~update_checksum,&rd_inf[8]);
                    //pbc_int32uToArray_bigEndian(~update_codesize,&rd_inf[12]);
                    //pbc_int32uToArray_bigEndian(update_srm,&rd_inf[16]);
                    pbc_int32uToArray_bigEndian(update_flag,&rd_inf[20]);
                    //pbc_int32uToArray_bigEndian(update_checksum,&rd_inf[24]);
                    //pbc_int32uToArray_bigEndian(update_codesize,&rd_inf[28]);
                    bsp_write_information_user_upgrade(&rd_inf[0]);       //写入信息到upgrade区

                    fwUpdate_status = fwUpdate_status_complete;
                }                
            }
            break;
        }
        default:
        {
            error = BGEUERR_Other;
            break;
        }
    }
    return(error);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//文件块比固件块大1
//-------------------------------------------------------------------------------------------------
sdt_bool mde_pull_upgrade_next_block(sdt_int16u* out_pBlock_num)
{
    *out_pBlock_num = (fwBlock_number+1);
    if(fwUpdate_status_complete == fwUpdate_status)
    {
        return(sdt_true);
    }
    return(sdt_false);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++