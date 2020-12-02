#ifndef med_watchdog_H
#define med_watchdog_H
//-----------------------------------------------------------------------------
#ifndef snail_data_types
    #include ".\depend\snail_data_types.h"
#endif
//-----------------------------------------------------------------------------
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name：看门狗配置
//入口:
//出口  
//----------------------------------------------------------------------------- 
void mde_watchdog_cfg(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称:喂狗
//入口:
//出口：  
//----------------------------------------------------------------------------- 
void mde_watchdog_reload(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++