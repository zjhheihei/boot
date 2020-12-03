//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include "hc32l17x.h"
void gpio_test(void)
{
    static sdt_bool cfg = sdt_false;
    static sdt_bool lighton;
    
    macro_createTimer(timer_light,timerType_millisecond,0);
    
    pbc_timerClockRun_task(&timer_light);
    if(pbc_pull_timerIsCompleted(&timer_light))
    {
       pbc_reload_timerClock(&timer_light,100);
       if(lighton)
       {
           lighton = sdt_false;
       }
       else
       {
           lighton = sdt_true;
       }
    }
    
    if(cfg)
    {
        if(lighton)
        {
            M0P_GPIO->PCOUT_f.PC13 = 1; 
        }
        else
        {
            M0P_GPIO->PCOUT_f.PC13 = 0;   
        }
    }
    else
    {
        cfg = sdt_true;
        M0P_SYSCTRL->PERI_CLKEN0_f.GPIO = 1;
        M0P_GPIO->PCDIR_f.PC13 = 0;       
        lighton = sdt_false;
    }


}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void app_bough_demo(void)
{
    mde_bough_link_task();
    app_upgrade_easy_task();
    

    if(pull_bough_recevie_one_message(0))
    {
        sdt_int8u local_addr[6] ={0x98,0x73,0x83,0x19,0x53,0x43};
        bgk_comm_buff_def* p_rx_bgk_buff;
        
        p_rx_bgk_buff = pull_bough_message_pBuff(0);
        switch(p_rx_bgk_buff->ProcotolType)
        {
            case BGPTCL_EasyUpagrde:
            {
                Bough_EasyUpgrade_Protocol(p_rx_bgk_buff,&local_addr[0]);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name: 通用应用task
//-----------------------------------------------------------------------------
void app_general_task(void)
{
    static sdt_bool sys_cfged = sdt_false;
    //mde_IWDG_FeedDog();
    if(sys_cfged)
    {
        gpio_test();
        mde_watchdog_reload();
        app_bough_demo();
        mde_random_task();
//        mde_hc595_task();    
//        app_sto_run_parameter_task();
//        app_led_task();
//        app_logic_realy_task();
//        app_master_comp_task();
//        app_master_sensor_task();
//        app_modbus_onewire_task();
//        app_modbus_pre_task();
//        app_modbus_remote_task();
//        app_pwm_control_task();
//        app_real_time_clock_task();
//        app_setpmotor_task();
//        app_temperature_task();
//        APP_BoughUpgradeProtocol_Task();
//        app_dac_control_task();
//        app_digti_in_task();
    }
    else
    {
        sys_cfged = sdt_true;      

        //app_read_run_parameter();      
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++