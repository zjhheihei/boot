//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "hc32l17x.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//flash map hc32l176kata
//128k flash,smallest unit 512b,256page
//0x0000 0000 -- 0x0001 FFFF
//boot
//0x0000 0000 -- 0x0000 0FFF     4k      8  page
//user.app 运行区
//0x0000 1000 -- 0x0000 FFFF     60k    120 page  
//user.upgrade 升级区
//0x0001 0000 -- 0x0001 EFFF     60k    120 page  
//reserve                    
//0x0001 F000 -- 0x0001 FFFF     4k      8  page 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void write_into_flash(sdt_int32u* in_pFlash_addr,sdt_int32u in_ram_data)@".wr_flash";
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define user_app_start_addr      0x00001000
#define user_upgrade_start_addr  0x00010000

#define user_app_inf_addr        0x0000FFE0
#define user_upgrade_inf_addr    0x0001EFE0
#define user_upgrade_end_addr    0x0001EFFF
#define page_unit_size           512
#define user_upgrade_inf_page    0x0001EE00
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//名称:读取app区的信息内容32bytes
//入口:返回的数据指针
//------------------------------------------------------------------------------
void bsp_read_information_user_app(sdt_int8u* out_pBuff)
{
    sdt_int8u *pAddr;
    sdt_int8u i;
    pAddr = (sdt_int8u*)user_app_inf_addr;
    for(i = 0;i < 32;i++)
    {
        *out_pBuff = *pAddr;
        pAddr ++;
        out_pBuff ++;
    }
}
//------------------------------------------------------------------------------
//名称:写入一个块的数据到upgrade 128bytes
//入口:块序号(从0开始)， 写入的数据指针
//------------------------------------------------------------------------------
void bsp_write_block_user_upgrade(sdt_int16u in_block_num,sdt_int8u* in_pBuff)
{
    sdt_int32u falsh_addr;
    sdt_int32u inf_backup[8];
    sdt_int32u inf_addr;
    sdt_bool inf_wtire_bk = sdt_false;
    sdt_int32u i;
    
    falsh_addr = user_upgrade_start_addr + in_block_num*128;
    
    if(0 == (falsh_addr%page_unit_size))  //page地址(512)
    {
        if(user_upgrade_inf_page == falsh_addr)  //最后一页
        {
            inf_addr = user_upgrade_inf_addr;
            for(i = 0;i < 8;i++)
            {
                inf_backup[i] = *(sdt_int32u*)inf_addr;
                inf_addr += 4;
            }
            inf_wtire_bk = sdt_true;
        }
        do
        {
            M0P_FLASH->BYPASS = 0x5A5A;      //unlock
            M0P_FLASH->BYPASS = 0xA5A5; 
            M0P_FLASH->CR_f.OP = 0x02;       //sector erase
        }while(0x02 != M0P_FLASH->CR_f.OP);

        //*((sdt_int8u*)falsh_addr) = 0x00;   //Erase  page
        write_into_flash((sdt_int32u*)falsh_addr,0);
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
    }
    
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;      //program
    }while(0x01 != M0P_FLASH->CR_f.OP);
    
    if(inf_wtire_bk)
    {
        inf_addr = user_upgrade_inf_addr;
        for(i = 0;i < 8;i++)
        {
            // *(sdt_int32u*)inf_addr = inf_backup[i];
            write_into_flash((sdt_int32u*)inf_addr,inf_backup[i]);
            inf_addr += 4;
            while(0 != M0P_FLASH->CR_f.BUSY)
            {
            }
        }
    }
    
    union
    {
        sdt_int8u inf_8bits[128];
        sdt_int32u inf_32bits[32];
    }inf_data;

    sdt_int32u words_32bits;
    //sdt_int32u i;

    for(i = 0;i < 128;i++)
    {
        inf_data.inf_8bits[i] = in_pBuff[i];
    }
    
    words_32bits = 32;         //128bytes 
    for(i=0;i<words_32bits;i++)
    {
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        //*(sdt_int32u*)falsh_addr = inf_data.inf_32bits[i];
        write_into_flash((sdt_int32u*)falsh_addr,inf_data.inf_32bits[i]);
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        falsh_addr += 4;
    }
}
//------------------------------------------------------------------------------
//名称:读取upgrade区的信息内容32bytes
//入口:返回的数据指针

void bsp_read_information_user_upgrade(sdt_int8u* out_pBuff)
{
    sdt_int8u *pAddr;
    sdt_int8u i;
    pAddr = (sdt_int8u*)user_upgrade_inf_addr;
    for(i = 0;i < 32;i++)
    {
        *out_pBuff = *pAddr;
        pAddr ++;
        out_pBuff ++;
    }
}
//------------------------------------------------------------------------------
//名称:写入upgrade区的信息内容32bytes
//入口:写入的数据指针

void bsp_write_information_user_upgrade(sdt_int8u* in_pBuff)
{
    sdt_int32u i;
    sdt_int32u backup[120];
    sdt_int32u falsh_addr;
    

    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK1 = 0xffffffff;   // lock boot and app sector
    }while(M0P_FLASH->SLOCK1 != 0xffffffff);

    falsh_addr = user_upgrade_inf_page;
    for(i = 0;i < 120;i++)
    {
        backup[i] = *(sdt_int32u*)falsh_addr;
        falsh_addr += 4;
    }
//------------------------------------------------------------------------------
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x02;       //sector erase
    }while(0x02 != M0P_FLASH->CR_f.OP);

    falsh_addr = user_upgrade_inf_page;
    //*((sdt_int8u*)falsh_addr) = 0x00;   //Erase inf page
    write_into_flash((sdt_int32u*)falsh_addr,0);
    while(0 != M0P_FLASH->CR_f.BUSY)
    {
    }
    
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;      //program
    }while(0x01 != M0P_FLASH->CR_f.OP);
//------------------------------------------------------------------------------
    falsh_addr = user_upgrade_inf_page;
    for(i = 0;i < 120;i++)
    {
      
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        //*(sdt_int32u*)falsh_addr = backup[i];
        write_into_flash((sdt_int32u*)falsh_addr,backup[i]);
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        falsh_addr += 4;
    }
//------------------------------------------------------------------------------
    union
    {
        sdt_int8u inf_8bits[32];
        sdt_int32u inf_32bits[8];
    }inf_data;

    sdt_int32u upgrade_falsh_addr;
    sdt_int32u words_32bits;


    for(i = 0;i < 32;i++)
    {
        inf_data.inf_8bits[i] = in_pBuff[i];
    }
    
    upgrade_falsh_addr = user_upgrade_inf_addr;
    words_32bits = 8;         //32bytes information
    for(i=0;i<words_32bits;i++)
    {
      
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        //*(sdt_int32u*)upgrade_falsh_addr = inf_data.inf_32bits[i];
        write_into_flash((sdt_int32u*)upgrade_falsh_addr,inf_data.inf_32bits[i]);
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        upgrade_falsh_addr += 4;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void write_into_flash(sdt_int32u* in_pFlash_addr,sdt_int32u in_ram_data)@".wr_flash"
{
    *in_pFlash_addr = in_ram_data;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++