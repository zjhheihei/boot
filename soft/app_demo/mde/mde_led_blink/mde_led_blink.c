//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\depend\snail_data_types.h"
#include "..\..\pbc\pbc_sysTick\pbc_sysTick.h"
#include ".\depend\bsp_led_blink.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++task++++++++++++++++++++++++++++++++++++++
void mde_led_blink_task(void)
{
    static sdt_bool cfg = sdt_false;
    static sdt_bool lighton;
    
    macro_createTimer(timer_light,timerType_millisecond,0);
    
    pbc_timerClockRun_task(&timer_light);
    if(pbc_pull_timerIsCompleted(&timer_light))
    {
       pbc_reload_timerClock(&timer_light,500);
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
            bsp_led_light_on();
        }
        else
        {
            bsp_led_light_off();
        }
    }
    else
    {
        cfg = sdt_true;
        bsp_blink_cfg();    
        lighton = sdt_false;
    }
}
//++++++++++++++++++++++++++++++++interface+++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++