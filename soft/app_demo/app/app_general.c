//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//bool digitIn1,digitIn2,digitIn3,digitIn4;
//
//void app_digit_input_task(void)
//{
//    digitIn1 = mde_gigit_pull_status(DIGITIN_1);
//    digitIn2 = mde_gigit_pull_status(DIGITIN_2);
//    digitIn3 = mde_gigit_pull_status(DIGITIN_3);
//    digitIn4 = mde_gigit_pull_status(DIGITIN_4);
//}

//void app_setpmotor_task(void)
//{
//    static bool cfged = false;
//    macro_createTimer(measure_delay,timerType_millisecond,0);
//    pbc_timerClockRun_task(&measure_delay);
//    if(pbc_pull_timerIsCompleted(&measure_delay))
//    {
//        pbc_reload_timerClock(&measure_delay,15000);
//        if(cfged)
//        {
//            cfged = false;
//            mde_FirstMotor_GotoOpen();
//        }
//        else
//        {
//            cfged = true;
//            mde_FirstMotor_GotoClose();
//        }
//    }
//}
#include "hc32l13x.h"
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
        M0P_SYSCTRL->PERI_CLKEN_f.GPIO = 1;
        M0P_GPIO->PCDIR_f.PC13 = 0;       
        lighton = sdt_false;
    }

//    volatile sdt_int32u i;
//    for(i=0;i < 500000;i++);
//    M0P_GPIO->PCOUT_f.PC13 = 0;
//    for(i=0;i < 500000;i++);
//    M0P_GPIO->PCOUT_f.PC13 = 1;    
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void app_bough_demo(void)
{
    mde_bough_link_task();
    app_upgrade_easy_task();
    macro_createTimer(timer_transmit,timerType_millisecond,0);
    
    pbc_timerClockRun_task(&timer_transmit);

    if(pull_bough_recevie_one_message(0))
    {
        sdt_int8u local_addr[6];
        bgk_comm_buff_def* p_rx_bgk_buff;
        p_rx_bgk_buff = pull_bough_message_pBuff(0);
        Bough_EasyUpgrade_Protocol(p_rx_bgk_buff,&local_addr[0]);
      /*
            static sdt_int16u length = 1;
            bgk_comm_buff_def* p_tx_bgk_buff;
            
            p_tx_bgk_buff = pull_bough_message_pBuff(0);
            p_tx_bgk_buff->LinkSrcAddr[0] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[1] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[2] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[3] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[4] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[5] = 0xee;
            
            p_tx_bgk_buff->LinkDstAddr[0] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[1] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[2] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[3] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[4] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[5] = 0xdd;     
            p_tx_bgk_buff->ProcotolType = 0xc000;
            p_tx_bgk_buff->PayloadLength = length;
            p_tx_bgk_buff->Payload[0] = 0x00;
            p_tx_bgk_buff->Payload[1] = 0x01;
            p_tx_bgk_buff->Payload[2] = 0x02;
            p_tx_bgk_buff->Payload[3] = 0x03;
            p_tx_bgk_buff->Payload[4] = 0x04;
            p_tx_bgk_buff->Payload[5] = 0x05;
            p_tx_bgk_buff->Payload[6] = 0x06;
            p_tx_bgk_buff->Payload[7] = 0x07;
            p_tx_bgk_buff->Payload[8] = 0x08;
            p_tx_bgk_buff->Payload[9] = 0x09;
            push_active_one_message_transmit(0,sdt_false);
            length ++;
      */
    }
    else
    {
        if(pbc_pull_timerIsCompleted(&timer_transmit))
        {
            pbc_reload_timerClock(&timer_transmit,1000);
            /*
            static sdt_int16u length = 1;
            bgk_comm_buff_def* p_tx_bgk_buff;
            
            p_tx_bgk_buff = pull_bough_message_pBuff(0);
            p_tx_bgk_buff->LinkSrcAddr[0] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[1] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[2] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[3] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[4] = 0xee;
            p_tx_bgk_buff->LinkSrcAddr[5] = 0xee;
            
            p_tx_bgk_buff->LinkDstAddr[0] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[1] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[2] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[3] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[4] = 0xdd;
            p_tx_bgk_buff->LinkDstAddr[5] = 0xdd;     
            p_tx_bgk_buff->ProcotolType = 0xc000;
            p_tx_bgk_buff->PayloadLength = length;
            p_tx_bgk_buff->Payload[0] = 0x00;
            p_tx_bgk_buff->Payload[1] = 0x01;
            p_tx_bgk_buff->Payload[2] = 0x02;
            p_tx_bgk_buff->Payload[3] = 0x03;
            p_tx_bgk_buff->Payload[4] = 0x04;
            p_tx_bgk_buff->Payload[5] = 0x05;
            p_tx_bgk_buff->Payload[6] = 0x06;
            p_tx_bgk_buff->Payload[7] = 0x07;
            p_tx_bgk_buff->Payload[8] = 0x08;
            p_tx_bgk_buff->Payload[9] = 0x09;
            push_active_one_message_transmit(0,sdt_true);
            length ++;
            */
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
        mde_watchdog_reload();
        gpio_test();
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