//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "stm32f10x.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//flash map stm32f103rct6
//256k flash,smallest unit 2k
//0x0800 0000 -- 0x0803 FFFF
//boot
//0x0800 0000 -- 0x0800 1FFF     8k      4  page
//user.app 运行区
//0x0800 2000 -- 0x0801 CFFF     108k    54  page  0-57page
//user.upgrade 升级区
//0x0801 D000 -- 0x0803 7FFF     108k    54  page  58,59,60,61,62-127page 由WRP3 bit7写保护
//reserve                    
//0x0803 8000 -- 0x0803 FFFF     32k     16  page 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define user_app_start_addr      0x08002000
#define user_upgrade_start_addr  0x0801D000
#define user_app_inf_addr        0x0801CFE0
#define user_upgrade_inf_addr    0x08037FE0
#define page_unit_size           2048
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef void (*pFunction)(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//程序跳转到应用区
//-----------------------------------------------------------------------------
void bsp_jump_to_user_app(void)
{
    pFunction Jump_To_Application;
    
    #ifdef NDEBUG
    IWDG_ReloadCounter(); 
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
//-----------------------------------------------------------------------------
void bsp_read_information_user_app(sdt_int8u* out_pInf)
{
    sdt_int8u *pAddr;
    sdt_int8u i;
    pAddr = (sdt_int8u*)user_app_inf_addr;
    for(i = 0;i < 32;i++)
    {
        *pAddr = *out_pInf;
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
        *pAddr = *out_pInf;
        pAddr ++;
        out_pInf ++;
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//读取flash的数据流
//-----------------------------------------------------------------------------
void bsp_read_4bytes_user_app(sdt_int32u in_offset_addr,sdt_int8u* out_pData)
{
    sdt_int8u* pAddr;
    sdt_int8u i;
    
    #ifdef NDEBUG
    IWDG_ReloadCounter();  //大数据传输，避免watchdog timeout
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
    IWDG_ReloadCounter();  //大数据传输，避免watchdog timeout
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
void bsp_transfer_user_upgrade_to_app(sdt_int32u in_codesize)
{
    sdt_int32u falsh_addr;
    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    
    #ifdef NDEBUG
    IWDG_ReloadCounter(); //
    #endif
    
    if((user_app_start_addr + in_codesize) > user_app_inf_addr)//地址保护
    {
        return;
    }
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_EraseOptionBytes(); 
    
    falsh_addr = user_app_start_addr;
    while(falsh_addr < user_upgrade_start_addr)
    {
        #ifdef NDEBUG
        IWDG_ReloadCounter(); //
        #endif

        FLASHStatus = FLASH_ErasePage(falsh_addr);   //Erase Appliction
        if(FLASHStatus == FLASH_COMPLETE)
        {
            falsh_addr += page_unit_size;
        }
        else
        {
            return;
        }
    }
    
    sdt_int32u app_falsh_addr;
    sdt_int32u upgrade_falsh_addr;
    sdt_int32u words_32bits;
    sdt_int32u i;
    
    app_falsh_addr = user_app_inf_addr;
    upgrade_falsh_addr = user_upgrade_inf_addr;
    words_32bits = 8;         //32bytes information
    for(i=0;i<words_32bits;i++)
    {
        #ifdef NDEBUG
        IWDG_ReloadCounter(); //
        #endif
        
        FLASHStatus = FLASH_ProgramWord(app_falsh_addr,*(sdt_int32u*)upgrade_falsh_addr);
        app_falsh_addr += 4;
        upgrade_falsh_addr += 4;
    }
    
    app_falsh_addr = user_app_start_addr;
    upgrade_falsh_addr = user_upgrade_start_addr;
    words_32bits = in_codesize/4;   //program data

    for(i=0;i<words_32bits;i++)
    {
        #ifdef NDEBUG
        IWDG_ReloadCounter(); //
        #endif
        
        FLASHStatus = FLASH_ProgramWord(app_falsh_addr,*(sdt_int32u*)upgrade_falsh_addr);
        app_falsh_addr += 4;
        upgrade_falsh_addr += 4;
    }

    //FLASHStatus=FLASH_EraseOptionBytes(); 
    FLASH_EnableWriteProtection(FLASH_WRProt_Pages0to1|FLASH_WRProt_Pages2to3|FLASH_WRProt_Pages4to5|FLASH_WRProt_Pages6to7|FLASH_WRProt_Pages8to9|\
                                FLASH_WRProt_Pages10to11|FLASH_WRProt_Pages12to13|FLASH_WRProt_Pages14to15|FLASH_WRProt_Pages16to17|\
                                FLASH_WRProt_Pages18to19|FLASH_WRProt_Pages20to21|FLASH_WRProt_Pages22to23|FLASH_WRProt_Pages24to25|\
                                FLASH_WRProt_Pages26to27|FLASH_WRProt_Pages28to29|FLASH_WRProt_Pages30to31|FLASH_WRProt_Pages32to33|\
                                FLASH_WRProt_Pages34to35|FLASH_WRProt_Pages36to37|FLASH_WRProt_Pages38to39|FLASH_WRProt_Pages40to41|\
                                FLASH_WRProt_Pages42to43|FLASH_WRProt_Pages44to45|FLASH_WRProt_Pages46to47|FLASH_WRProt_Pages48to49|\
                                FLASH_WRProt_Pages50to51|FLASH_WRProt_Pages52to53|FLASH_WRProt_Pages54to55|FLASH_WRProt_Pages56to57\
                                );
    FLASH_Lock();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++