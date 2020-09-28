//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _BSP_STORAGE_H
#define _BSP_STORAGE_H
//-----------------------------------------------------------------------------
#ifndef snail_data_types
    #include ".\snail_data_types.h"
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_transfer_user_upgrade_to_app(sdt_int32u in_codesize);
//-----------------------------------------------------------------------------
//读取4个字节的数据
void bsp_read_4bytes_user_app(sdt_int32u in_offset_addr,sdt_int8u* out_pData);
void bsp_read_4bytes_user_upgrade(sdt_int32u in_offset_addr,sdt_int8u* out_pData);
//-----------------------------------------------------------------------------
//读取升级信息32bits
void bsp_read_information_user_app(sdt_int8u* out_pInf);
void bsp_read_information_user_upgrade(sdt_int8u* out_pInf);
//-----------------------------------------------------------------------------
void bsp_jump_to_user_app(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++