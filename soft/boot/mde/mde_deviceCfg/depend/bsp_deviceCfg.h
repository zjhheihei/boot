//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _BSP_DEVICECFG_H
#define _BSP_DEVICECFG_H
//-----------------------------------------------------------------------------
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_sysclock_cfg(void);
//-----------------------------------------------------------------------------
void bsp_watchdog_cfg(void);
//-----------------------------------------------------------------------------
void bsp_watchdog_reload(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++