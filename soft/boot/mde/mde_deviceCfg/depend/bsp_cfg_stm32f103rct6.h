//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "stm32f10x.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_sysclock_cfg(void)
{
    static sdt_int32u count=0;
    
    __disable_interrupt();
    for(;count<50000;count++)
    {
        IWDG_ReloadCounter();
        __no_operation();  
    }
    RCC_DeInit();                        //系统时钟复位
    RCC_HSEConfig(RCC_HSE_ON);           //打开晶体振荡器
    if(ERROR==RCC_WaitForHSEStartUp())
    {
        while(1);                           //晶体出错,等待复位
    }
        
    FLASH->ACR |= FLASH_ACR_PRFTBE;                          //开启指令预取功能/* Enable Prefetch Buffer */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);  //指令等待2周期 /* Flash 2 wait state */
    FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;    
    
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_6);  //PLL配置
    RCC_PLLCmd(ENABLE);                                 //打开PLL
    while((RCC->CR & RCC_CR_PLLRDY) == 0)               //等待PLL稳定
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);          //切换到PLL时钟
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
    {
    }                                           //等待PLL成为系统时钟源
    RCC_HCLKConfig(RCC_SYSCLK_Div1);            //AHB时钟,
    RCC_PCLK1Config(RCC_HCLK_Div2);             //APB1时钟,AHB时钟2分频
    RCC_PCLK2Config(RCC_HCLK_Div1);             //APB2时钟,
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define LsiFreq   40000
//-----------------------------------------------------------------------------
void bsp_watchdog_cfg(void)
{
    RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {}
  
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);

    /* Set counter reload value to obtain 1000ms IWDG TimeOut.
       Counter Reload Value = 1000ms/IWDG counter clock period
                            = 1000ms / (LSI/32)
                            = 1s / (LsiFreq/32)
                            = LsiFreq/(32 * 1)
                            = LsiFreq/32
     */
    IWDG_SetReload(LsiFreq/32);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}
//-----------------------------------------------------------------------------
void bsp_watchdog_reload(void)
{
    IWDG_ReloadCounter();
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++