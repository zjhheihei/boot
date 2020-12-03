//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "chip_select.h"
#include ".\snail_data_types.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if defined(stm32f103rct6)
#include ".\bsp_cfg_stm32f103rct6.h"
#elif defined(stm8l052r8t6)
#include ".\bsp_cfg_stm8l052r8t6.h"
#elif defined(hc32l136k8ta)
#include ".\bsp_cfg_hc32l136k8ta.h"
#elif defined(hc32l176kata)
#include ".\bsp_cfg_hc32l176kata.h"
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++