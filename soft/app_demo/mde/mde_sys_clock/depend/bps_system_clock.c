//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
#include "hc32l13x.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum en_sysctrl_clk_source
{
    SysctrlClkRCH  = 0u,               ///< 内部高速时钟
    SysctrlClkXTH  = 1u,               ///< 外部高速时钟
    SysctrlClkRCL  = 2u,               ///< 内部低速时钟
    SysctrlClkXTL  = 3u,               ///< 外部低速时钟
    SysctrlClkPLL  = 4u,               ///< PLL时钟
}en_sysctrl_clk_source_t;
/**
 *******************************************************************************
 ** \brief HCLK时钟分频系数类型定义
 ******************************************************************************/
typedef enum en_sysctrl_hclk_div
{
    SysctrlHclkDiv1   = 0u,              ///< SystemClk
    SysctrlHclkDiv2   = 1u,              ///< SystemClk/2
    SysctrlHclkDiv4   = 2u,              ///< SystemClk/4
    SysctrlHclkDiv8   = 3u,              ///< SystemClk/8
    SysctrlHclkDiv16  = 4u,              ///< SystemClk/16
    SysctrlHclkDiv32  = 5u,              ///< SystemClk/32
    SysctrlHclkDiv64  = 6u,              ///< SystemClk/64
    SysctrlHclkDiv128 = 7u,              ///< SystemClk/128
}en_sysctrl_hclk_div_t;

/**
 *******************************************************************************
 ** \brief PCLK分频系数
 ******************************************************************************/
typedef enum en_sysctrl_pclk_div
{
    SysctrlPclkDiv1 = 0u,                ///< HCLK
    SysctrlPclkDiv2 = 1u,                ///< HCLK/2
    SysctrlPclkDiv4 = 2u,                ///< HCLK/4
    SysctrlPclkDiv8 = 3u,                ///< HCLK/8
}en_sysctrl_pclk_div_t;
//-----------------------------------------------------------------------------
#define RC_TRIM_BASE_ADDR           ((volatile uint16_t*)   (0x00100C00ul))
#define RCH_CR_TRIM_24M_VAL         (*((volatile uint16_t*) (0x00100C00ul)))
#define RCH_CR_TRIM_22_12M_VAL      (*((volatile uint16_t*) (0x00100C02ul)))
#define RCH_CR_TRIM_16M_VAL         (*((volatile uint16_t*) (0x00100C04ul)))
#define RCH_CR_TRIM_8M_VAL          (*((volatile uint16_t*) (0x00100C06ul)))
#define RCH_CR_TRIM_4M_VAL          (*((volatile uint16_t*) (0x00100C08ul)))

#define RCL_CR_TRIM_38400_VAL       (*((volatile uint16_t*) (0x00100C20ul)))
#define RCL_CR_TRIM_32768_VAL       (*((volatile uint16_t*) (0x00100C22ul)))
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:HC32L136时钟配置
//功能:配置晶振、FLASH等待时间、系统时钟
//RCH 24M
//入口:无,在系统复位后,第一个调用
//出口:无
//-----------------------------------------------------------------------------
void bsp_clock_cfg(void)
{
    #ifdef NDEBUG
    *(volatile uint32_t*)0xE000ED08 = 0x00000800;  //app_vect_table
    #endif
    M0P_SYSCTRL->PERI_CLKEN_f.FLASH = 1;
    //时钟切换到RCL                                    
    M0P_SYSCTRL->RCL_CR_f.TRIM = RCL_CR_TRIM_38400_VAL;
    M0P_SYSCTRL->RCL_CR_f.STARTUP = 0x01;   //16个周期
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.RCL_EN = 1;
    while(1 != M0P_SYSCTRL->RCL_CR_f.STABLE)
    {
    }
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.CLKSW = SysctrlClkRCL;
    
    M0P_FLASH->BYPASS = 0x5A5A;
    M0P_FLASH->BYPASS = 0xA5A5;
    M0P_FLASH->CR_f.WAIT = 0;  //flash 等待时间
    
    //时钟切换到RCH
    M0P_SYSCTRL->RCH_CR_f.TRIM = RCH_CR_TRIM_24M_VAL;  //24M
    if(1 != M0P_SYSCTRL->SYSCTRL0_f.RCH_EN)
    {
        M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
        M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
        M0P_SYSCTRL->SYSCTRL0_f.RCH_EN = 1;
    }
    while(1 != M0P_SYSCTRL->RCH_CR_f.STABLE)
    {
    }
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.CLKSW = SysctrlClkRCH;
   // Flash_WaitCycle(FlashWaitCycle0);       //flash 等待时间
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.RCL_EN = 0;     //关闭RCL
    
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.HCLK_PRS = SysctrlHclkDiv1;  //HCLK 
      
    M0P_SYSCTRL->SYSCTRL2 = 0x5A5A;         //unlock
    M0P_SYSCTRL->SYSCTRL2 = 0xA5A5;
    M0P_SYSCTRL->SYSCTRL0_f.PCLK_PRS = SysctrlPclkDiv1;  //PCLK

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
