//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "hc32l13x.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//flash map hc32l136k8ta
//64k flash,smallest unit 512b,128page
//0x0000 0000 -- 0x0000 FFFF
//boot
//0x0000 0000 -- 0x0000 07FF     2k      4  page
//user.app 运行区
//0x0000 0800 -- 0x0000 7FFF     30k    60  page  
//user.upgrade 升级区
//0x0000 8000 -- 0x0000 F7FF     30k    60  page  
//reserve                    
//0x0000 F800 -- 0x0000 FFFF     2k     4   page 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define user_app_start_addr      0x00000800
#define user_upgrade_start_addr  0x00008000

#define user_app_inf_addr        0x00007FE0
#define user_upgrade_inf_addr    0x0000FFE0
#define page_unit_size           512
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef void (*pFunction)(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//程序跳转到应用区
//-----------------------------------------------------------------------------
#define KR_KEY_Reload    ((uint16_t)0xAAAA)
//-----------------------------------------------------------------------------
void bsp_jump_to_user_app(void)
{
    pFunction Jump_To_Application;
    
    #ifdef NDEBUG
    M0P_WDT->RST = 0x1E;
    M0P_WDT->RST = 0xE1;
    #endif
      
    __disable_interrupt();  
    /* Jump to user application */
    Jump_To_Application = (pFunction)(*(__IO uint32_t*)(user_app_start_addr + 4));
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) user_app_start_addr);
    Jump_To_Application(); 
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//32byte的信息数据
void bsp_transfer_user_upgrade_to_app(sdt_int32u in_codesize);
//-----------------------------------------------------------------------------
void bsp_read_information_user_app(sdt_int8u* out_pInf)
{
    sdt_int8u *pAddr;
    sdt_int8u i;
    pAddr = (sdt_int8u*)user_app_inf_addr;
    for(i = 0;i < 32;i++)
    {
        *out_pInf = *pAddr;
        pAddr ++;
        out_pInf ++;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_read_information_user_upgrade(sdt_int8u* out_pInf)
{
    sdt_int8u *pAddr;
    sdt_int8u i;
    pAddr = (sdt_int8u*)user_upgrade_inf_addr;
    for(i = 0;i < 32;i++)
    {
        *out_pInf = *pAddr;
        pAddr ++;
        out_pInf ++;
    }
    //bsp_transfer_user_upgrade_to_app(30688); //test
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//读取flash的数据流
//-----------------------------------------------------------------------------
void bsp_read_4bytes_user_app(sdt_int32u in_offset_addr,sdt_int8u* out_pData)
{
    sdt_int8u* pAddr;
    sdt_int8u i;
    
    #ifdef NDEBUG
    M0P_WDT->RST = 0x1E;
    M0P_WDT->RST = 0xE1; //大数据传输，避免watchdog timeout
    #endif
    
    if((user_app_start_addr + in_offset_addr) > user_app_inf_addr)//地址保护
    {
        return;
    }
    pAddr = (sdt_int8u*)(user_app_start_addr + in_offset_addr);
    for(i = 0;i < 4;i++)
    {
        *out_pData = *pAddr;
        out_pData++;
        pAddr++;
    }

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_read_4bytes_user_upgrade(sdt_int32u in_offset_addr,sdt_int8u* out_pData)
{
    sdt_int8u* pAddr;
    sdt_int8u i;
    
    #ifdef NDEBUG
    M0P_WDT->RST = 0x1E;
    M0P_WDT->RST = 0xE1; //大数据传输，避免watchdog timeout
    #endif
    
    if((user_upgrade_start_addr + in_offset_addr) > user_upgrade_inf_addr)//地址保护
    {
        return;
    }
    pAddr = (sdt_int8u*)(user_upgrade_start_addr + in_offset_addr);
    for(i = 0;i < 4;i++)
    {
        *out_pData = *pAddr;
        out_pData++;
        pAddr++;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_transfer_user_upgrade_to_app(sdt_int32u in_codesize)
{
    sdt_int32u falsh_addr;
    
    #ifdef NDEBUG
    M0P_WDT->RST = 0x1E;
    M0P_WDT->RST = 0xE1; //大数据传输，避免watchdog timeout
    #endif
    
    if((user_app_start_addr + in_codesize) > user_app_inf_addr)//地址保护
    {
        return;
    }
    
    M0P_FLASH->BYPASS  = 0x5A5A;//配置24M的flash操作时间
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TNVS    = 0xC0;
    
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TPGS    = 0x8A;
    
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TPROG   = 0xA2;
    
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TSERASE = 0x1A5E0;
    
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TMERASE = 0xCD140;
    
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TPRCV   = 0x90;

    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TSRCV   = 0x5A0;
        
    M0P_FLASH->BYPASS  = 0x5A5A;
    M0P_FLASH->BYPASS  = 0xA5A5; 
    M0P_FLASH->TMRCV   = 0x1770;
    
    #ifdef NDEBUG
    #else//test
    sdt_int32u test_addr;
    sdt_int32u test_reg;
    test_addr = user_app_inf_addr - 4; 
    test_reg = *(sdt_int32u*)test_addr + 1;
    #endif
    
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0xfffffffe;   //only lock boot's sector
    }while(M0P_FLASH->SLOCK != 0xfffffffe);
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x02;       //sector erase
    }while(0x02 != M0P_FLASH->CR_f.OP);

    falsh_addr = user_app_start_addr;
    while(falsh_addr < user_upgrade_start_addr)  //erase user.app 
    {
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1;
        #endif
        
        *((sdt_int8u*)falsh_addr) = 0x00;   //Erase Appliction
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        falsh_addr += page_unit_size;
    }
    
    #ifdef NDEBUG
    #else//test
    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;      //program
    }while(0x01 != M0P_FLASH->CR_f.OP);
    *(sdt_int32u*)test_addr = test_reg; 
    #endif
    
    
    sdt_int32u app_falsh_addr;
    sdt_int32u upgrade_falsh_addr;
    sdt_int32u words_32bits;
    sdt_int32u i;

    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5; 
        M0P_FLASH->CR_f.OP = 0x01;      //program
    }while(0x01 != M0P_FLASH->CR_f.OP);
    
    app_falsh_addr = user_app_inf_addr;
    upgrade_falsh_addr = user_upgrade_inf_addr;
    words_32bits = 8;         //32bytes information
    for(i=0;i<words_32bits;i++)
    {
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        *(sdt_int32u*)app_falsh_addr = *(sdt_int32u*)upgrade_falsh_addr;
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        app_falsh_addr += 4;
        upgrade_falsh_addr += 4;
    }
    
    app_falsh_addr = user_app_start_addr;
    upgrade_falsh_addr = user_upgrade_start_addr;
    words_32bits = in_codesize/4;   //program data

    for(i=0;i<words_32bits;i++)
    {
        #ifdef NDEBUG
        M0P_WDT->RST = 0x1E;
        M0P_WDT->RST = 0xE1; 
        #endif
        
        //*(sdt_int32u*)app_falsh_addr = *(sdt_int32u*)upgrade_falsh_addr + i +1;  //test
        *(sdt_int32u*)app_falsh_addr = *(sdt_int32u*)upgrade_falsh_addr;
        while(0 != M0P_FLASH->CR_f.BUSY)
        {
        }
        app_falsh_addr += 4;
        upgrade_falsh_addr += 4;
    }

    do
    {
        M0P_FLASH->BYPASS = 0x5A5A;      //unlock
        M0P_FLASH->BYPASS = 0xA5A5;    
        M0P_FLASH->SLOCK = 0xffff0000;   // lock boot and app sector
    }while(M0P_FLASH->SLOCK != 0xffff0000);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++