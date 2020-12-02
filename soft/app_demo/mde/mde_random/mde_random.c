//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\depend\snail_data_types.h"
#include ".\depend\bsp_random.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//产生5组50--2000之间的随机数
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    randrs_idle         = 0x00,
    randrs_start_trng   = 0x01,
    randrs_tring        = 0x02,
    randrs_wait         = 0x03,
}random_run_s_def;
//------------------------------------------------------------------------------
typedef struct
{
    random_run_s_def  randop_rs;
    sdt_int16u random_buff[5];
    sdt_int8u  index;
}random_oper_def;
//------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static random_oper_def  random_oper;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void push_one_random_into_buff(sdt_int16u in_rand_data)
{
    if(random_oper.index < 5)
    {
        random_oper.random_buff[random_oper.index] = in_rand_data;
        random_oper.index ++;
    }
}
//------------------------------------------------------------------------------
static sdt_int16u pull_one_random_from_buff(void)
{
    sdt_int16u random_data = 2000;
    sdt_int8u i;
    
    if(0 != random_oper.index)
    {
        random_data = random_oper.random_buff[0];
        
        for(i = 0;i < random_oper.index;i++)
        {
            if(i < (random_oper.index-1))
            {
                random_oper.random_buff[i] = random_oper.random_buff[i+1];
            }
            else
            {
                random_oper.random_buff[i] = 0;
            }
        }
        random_oper.index --;
    }
    return(random_data);
}
//++++++++++++++++++++++++++++++++++++task++++++++++++++++++++++++++++++++++++++
void mde_random_task(void)
{
    static sdt_bool cfged = sdt_false;
    
    if(cfged)
    {
        bsp_trng_task();
        switch(random_oper.randop_rs)
        {
            case randrs_idle:
            {
                random_oper.randop_rs = randrs_start_trng;
                break;
            }
            case randrs_start_trng:
            {
                 bsp_trng_run_once();
                 random_oper.randop_rs = randrs_tring;
                break;
            }
            case randrs_tring:
            {
                if(bps_pull_trng_complete())
                {
                    push_one_random_into_buff(bsp_pull_random_data());
                    random_oper.randop_rs = randrs_wait;
                }
                break;
            }
            case randrs_wait:
            {
                if(random_oper.index < 5)
                {
                    random_oper.randop_rs = randrs_start_trng;
                }
                break;
            }
            default:
            {
                random_oper.randop_rs = randrs_idle;
                break;
            }
        }
    }
    else
    {
        cfged = sdt_true;
        bsp_random_cfg();
    }
}
//++++++++++++++++++++++++++++++++interface+++++++++++++++++++++++++++++++++++++
sdt_int16u pull_mde_one_random_data(void)
{
    return(pull_one_random_from_buff());
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++