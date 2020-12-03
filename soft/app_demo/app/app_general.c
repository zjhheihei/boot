//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    
    mde_watchdog_reload();
    if(sys_cfged)
    {
        mde_led_blink_task();
        
        app_bough_demo();
        mde_random_task();

    }
    else
    {
        sys_cfged = sdt_true;      

        //app_read_run_parameter();      
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++