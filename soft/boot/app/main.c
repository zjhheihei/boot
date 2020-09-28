//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\app_cfg.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void bootload_task(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main (void)
{
    mde_device_system_cfg();
//-----------------------------------------------------------------------------
    while(1)
    {
        bootload_task();
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Bootload应用代码,管理升级和启动，以及应用区的代码检查
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    start                  =0x00,   //开始
    checkAndUpdating       =0x01,   //检查是否需要升级
    checkLegal             =0x02,   //合法性检测
    failedLegal            =0x03,   //在Boot区运行
}bootStatus_Def;
//-----------------------------------------------------------------------------
static bootStatus_Def bootStatus = start;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void bootload_task(void)
{
    static sdt_int8u check_cnt = 0;
    
    switch(bootStatus)
    {
        case start:
        {
            bootStatus = checkAndUpdating;
            break;
        }
        case checkAndUpdating:
        {
            mde_need_upgrade();
            bootStatus = checkLegal;
            break;
        }
        case checkLegal:
        {
            if(mde_user_app_is_legal())
            {
                #ifdef NDEBUG
                mde_watchdog_reload();
                mde_jump_to_user_app();
                #else
                mde_jump_to_user_app();
                while(1);
                #endif                
            }
            else
            {
                bootStatus = failedLegal;
            }
            break;
        }
        case failedLegal:
        {
            if(check_cnt < 3)
            {
                check_cnt ++;
                bootStatus = checkAndUpdating;
            }
            break;
        }
        default:
        {
            bootStatus = start;
            break;
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++