//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _BSP_BOUGHLINK_H
#define _BSP_BOUGHLINK_H
//------------------------------------------------------------------------------
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
//Task
//------------------------------------------------------------------------------
//Event
//------------------------------------------------------------------------------
//Status
//------------------------------------------------------------------------------
//Function
//------------------------------------------------------------------------------
void Uart2_Configure(void);
//------------------------------------------------------------------------------
void Recfg_Buadrate_USART3(sdt_int32u ClockFreqValue);
//------------------------------------------------------------------------------
sdt_bool Pull_PHYTxd_Finish(void);
//------------------------------------------------------------------------------
void Enter_ReceiveMode_PHY(void);
//------------------------------------------------------------------------------
void Enter_TransmitMode_PHY(void);
//------------------------------------------------------------------------------
sdt_bool Pull_OneByteFromPHY(sdt_int8u* _Out_Byte);
//------------------------------------------------------------------------------
sdt_bool Pull_TransmitPHY_IsReady(void);
//------------------------------------------------------------------------------
void Push_OneByteToPHY(sdt_int8u _In_Byte);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_uart0_cfg(void);
void bsp_look_for_byte_rx_uart0(void);
sdt_int16u bsp_transfet_bytes_tx_uart0(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes,sdt_bool in_conflict_check);
sdt_bool bsp_pull_complete_tx_uart0(void);
sdt_bool bsp_pull_pyh_busy_uart0(void);
void bsp_phy_baudrate_calibrate_uart0(void);
void bsp_entry_phy_rx_uart0(void);
void bsp_entry_phy_tx_uart0(void);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//++++++++++++++++++++++++++++++++++End+++++++++++++++++++++++++++++++++++++++++