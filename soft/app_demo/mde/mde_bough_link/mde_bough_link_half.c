//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
#include ".\depend\bsp_bough_link.h"
#include ".\mde_bough_link_half.h"
#include "..\..\pbc\pbc_sysTick\pbc_sysTick.h"
#include "..\mde_sys_clock\mde_system_clock.h"
#include "..\..\pbc\pbc_crc16\pbc_crc16.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//------------------------------------------------------------------------------
#define SYB_PREAMBLE    0x55
#define SYB_SFD         0xAB
//------------------------------------------------------------------------------
#define TIMEOUTV        1000
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Bough 协议链路层处理
//支持多模块化数据处理
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    bgk_rs_idle            = 0x00,  //
    bgk_rs_rx_preamble     = 0x01,  //接收到头
    bgk_rs_rx_sfd          = 0x02,  //接收界定符
    bgk_rs_rx_lenHi        = 0x03,  //数据包长度  
    bgk_rs_rx_lenLo        = 0x04,
    bgk_rs_rx_data         = 0x05,  //数据包
    bgk_rs_rx_complete     = 0x06,  //完成
    
    bgk_rs_tx_start        = 0x08,  
    bgk_rs_tx_doing        = 0x09,  //发送
    bgk_rs_tx_complete     = 0x0a,  //    
}bgk_run_status_def;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define bgk_bits_message_rx     0x0001 //新报文生产
#define bgk_bits_rx_byte        0x0002 //收一字节数据
#define bgk_bits_tx_conflict    0x0004 //发送冲突检测模式
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct
{
    sdt_int8u           rx_byte;  //接收的字符
    sdt_int16u          bgk_bits; //状态字
    bgk_comm_buff_def   bgk_comm_buff;
    bgk_trans_mon_def   transmit_monitor;
    bgk_run_status_def  bgk_run_status;
    sdt_int8u           bgk_rx_buff[MAX_REV_BUFFER];
    sdt_int16u          receive_len;
    sdt_int16u          receive_index;
    sdt_int8u           bgk_tx_buff[MAX_TRN_BUFFER];
    sdt_int16u          transmit_len;
    sdt_int16u          transmit_index;
    sdt_int16u          tx_feedback_index;//回环数据指针
    timerClock_def      timer_rx_timeout;    
    timerClock_def      timer_tx_timeout;  
    timerClock_def      timer_brate_cal;//波特率校准间隔

    void (*pull_look_for_byte_rx)(void);
    sdt_int16u (*transfet_bytes_to_phy_tx)(sdt_int8u* in_pByte,sdt_int16u in_expect_bytes,sdt_bool in_conflict_check);
    sdt_bool (*pull_complete_tx_data)(void);
    sdt_bool (*pull_phy_busy)(void);
    void(*phy_baudrate_calibrate)(void);
    void (*entry_phy_rx)(void);
    void (*entry_phy_tx)(void);
}bgk_oper_def;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//数据状态处理
//------------------------------------------------------------------------------
static void bough_link_state_jump(bgk_oper_def* in_pBgk_oper)
{
    bgk_run_status_def rd_bgk_run_status;
    
    pbc_timerClockRun_task(&in_pBgk_oper->timer_brate_cal);
    
    pbc_timerMillRun_task(&in_pBgk_oper->timer_rx_timeout);
    pbc_timerMillRun_task(&in_pBgk_oper->timer_tx_timeout);
    
    if(pbc_pull_timerIsOnceTriggered(&in_pBgk_oper->timer_rx_timeout))
    {
        in_pBgk_oper->bgk_run_status = bgk_rs_idle;
    }    
    if(pbc_pull_timerIsOnceTriggered(&in_pBgk_oper->timer_tx_timeout))
    {
        in_pBgk_oper->bgk_run_status = bgk_rs_idle;
        in_pBgk_oper->transmit_monitor = bgk_trans_mon_error;
    }

    do
    {
        rd_bgk_run_status = in_pBgk_oper->bgk_run_status;
        switch(rd_bgk_run_status)
        {
            case bgk_rs_idle:
            {
                pbc_reload_timerClock(&in_pBgk_oper->timer_brate_cal,120); //120s校准一次
                in_pBgk_oper->phy_baudrate_calibrate();
                in_pBgk_oper->entry_phy_rx();
                in_pBgk_oper->bgk_run_status = bgk_rs_rx_preamble;
                break;
            }
            case bgk_rs_rx_preamble:
            {
                if(pbc_pull_timerIsCompleted(&in_pBgk_oper->timer_brate_cal))
                {
                    pbc_reload_timerClock(&in_pBgk_oper->timer_brate_cal,120);
                    in_pBgk_oper->phy_baudrate_calibrate();
                }
                if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)
                {
                    in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                    if(SYB_PREAMBLE == in_pBgk_oper->rx_byte)
                    {
                        in_pBgk_oper->bgk_run_status = bgk_rs_rx_sfd;
                        pbc_reload_timerClock(&in_pBgk_oper->timer_rx_timeout,TIMEOUTV);
                    }  
                }
                break;
            }
            case bgk_rs_rx_sfd:
            {
                if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)
                {
                    in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                    if(SYB_SFD == in_pBgk_oper->rx_byte)
                    {
                        in_pBgk_oper->bgk_run_status = bgk_rs_rx_lenHi;
                        pbc_reload_timerClock(&in_pBgk_oper->timer_rx_timeout,TIMEOUTV);
                    }  
                }
                break;
            }
            case bgk_rs_rx_lenHi:
            {
                if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)
                {
                    in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                    in_pBgk_oper->bgk_rx_buff[0] = in_pBgk_oper->rx_byte;
                    in_pBgk_oper->receive_len = ((sdt_int16u)in_pBgk_oper->rx_byte) << 8;
                    in_pBgk_oper->bgk_run_status = bgk_rs_rx_lenLo;
                }
                break;
            }
            case bgk_rs_rx_lenLo:
            {
                if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)
                {
                    in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                    in_pBgk_oper->receive_len |= ((sdt_int16u)in_pBgk_oper->rx_byte)&0x00ff;
                    if(in_pBgk_oper->receive_len > MAX_REV_BUFFER)//长度溢出
                    {
                        in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                        pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_rx_timeout);
                    }
                    else if(in_pBgk_oper->receive_len < 18)
                    {
                        in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                        pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_rx_timeout);
                    }
                    else
                    {
                        in_pBgk_oper->bgk_run_status = bgk_rs_rx_data;
                        in_pBgk_oper->bgk_rx_buff[1] = in_pBgk_oper->rx_byte;
                        in_pBgk_oper->receive_index = 2;
                    }
                    
                }
                break;
            }
            case bgk_rs_rx_data:
            {
                if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)
                {
                    in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                    
                    in_pBgk_oper->bgk_rx_buff[in_pBgk_oper->receive_index] = in_pBgk_oper->rx_byte;
                    if((in_pBgk_oper->receive_index + 1) >= in_pBgk_oper->receive_len)
                    {
                        sdt_int8u Calculate_CRC[2];
                        Crc16CalculateOfByte(&in_pBgk_oper->bgk_rx_buff[0],(in_pBgk_oper->receive_len - 2),&Calculate_CRC[0]);
                        if((Calculate_CRC[0] == in_pBgk_oper->bgk_rx_buff[in_pBgk_oper->receive_len - 2]) &&\
                           (Calculate_CRC[1] == in_pBgk_oper->bgk_rx_buff[in_pBgk_oper->receive_len - 1]))
                        {
                            in_pBgk_oper->bgk_comm_buff.ProcotolType = in_pBgk_oper->bgk_rx_buff[2];
                            in_pBgk_oper->bgk_comm_buff.ProcotolType = in_pBgk_oper->bgk_comm_buff.ProcotolType << 8;
                            in_pBgk_oper->bgk_comm_buff.ProcotolType |= (sdt_int16u)in_pBgk_oper->bgk_rx_buff[3] & 0x00FF;
                            
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[0] =  in_pBgk_oper->bgk_rx_buff[4];
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[1] =  in_pBgk_oper->bgk_rx_buff[5];
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[2] =  in_pBgk_oper->bgk_rx_buff[6];
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[3] =  in_pBgk_oper->bgk_rx_buff[7];
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[4] =  in_pBgk_oper->bgk_rx_buff[8];
                            in_pBgk_oper->bgk_comm_buff.LinkDstAddr[5] =  in_pBgk_oper->bgk_rx_buff[9];

                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[0] =  in_pBgk_oper->bgk_rx_buff[10];
                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[1] =  in_pBgk_oper->bgk_rx_buff[11];
                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[2] =  in_pBgk_oper->bgk_rx_buff[12];
                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[3] =  in_pBgk_oper->bgk_rx_buff[13];
                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[4] =  in_pBgk_oper->bgk_rx_buff[14];
                            in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[5] =  in_pBgk_oper->bgk_rx_buff[15];
                            
                            in_pBgk_oper->bgk_comm_buff.PayloadLength = in_pBgk_oper->receive_len - 18;
                            sdt_int16u i;
                            for(i = 0;i < (in_pBgk_oper->bgk_comm_buff.PayloadLength);i++)
                            {
                                in_pBgk_oper->bgk_comm_buff.Payload[i] = in_pBgk_oper->bgk_rx_buff[i+16];
                            }
                            in_pBgk_oper->bgk_bits |= bgk_bits_message_rx;  //产生一个报文
                            in_pBgk_oper->bgk_run_status = bgk_rs_rx_complete;
                        }
                        else
                        {
                            in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                        }
                        pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_rx_timeout);
                    }
                    in_pBgk_oper->receive_index ++;
                }
                break;
            }
            case bgk_rs_rx_complete:
            {
                if(in_pBgk_oper->bgk_bits & bgk_bits_message_rx)
                {
                    if(bgk_bits_rx_byte & in_pBgk_oper->bgk_bits)  
                    {
                        in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                        if(SYB_PREAMBLE == in_pBgk_oper->rx_byte)
                        {
                            in_pBgk_oper->bgk_run_status = bgk_rs_rx_sfd;  //如果有持续数据，进行新一轮的接收
                            pbc_reload_timerClock(&in_pBgk_oper->timer_rx_timeout,TIMEOUTV);
                        }  
                    }
                }
                else
                {
                    in_pBgk_oper->bgk_run_status = bgk_rs_idle;//数据被读取，转入空闲
                }
                break;
            }
            case bgk_rs_tx_start:
            {
                
                sdt_int16u Data_Length;
                sdt_int16u PandSFD_Index = 0;
    
                while(PandSFD_Index < (PREABBLE_LEN+1))//encode
                {
                    if(PandSFD_Index == PREABBLE_LEN)
                    {
                        in_pBgk_oper->bgk_tx_buff[PandSFD_Index] = SYB_SFD;
                    }
                    else
                    {
                        in_pBgk_oper->bgk_tx_buff[PandSFD_Index] = SYB_PREAMBLE;
                    }
                    PandSFD_Index++;
                }

                Data_Length = in_pBgk_oper->bgk_comm_buff.PayloadLength + 18;
                if((PandSFD_Index+Data_Length) > MAX_TRN_BUFFER)
                {
                    in_pBgk_oper->transmit_monitor = bgk_trans_mon_error;
                    in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                    pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_tx_timeout);
                }
                else
                {
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+0] = Data_Length>>8;
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+1] = Data_Length;

                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+2] = in_pBgk_oper->bgk_comm_buff.ProcotolType>>8;
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+3] = in_pBgk_oper->bgk_comm_buff.ProcotolType;

                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+4] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[0];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+5] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[1];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+6] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[2];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+7] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[3];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+8] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[4];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+9] = in_pBgk_oper->bgk_comm_buff.LinkDstAddr[5];

                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+10] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[0];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+11] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[1];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+12] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[2];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+13] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[3];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+14] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[4];
                    in_pBgk_oper->bgk_tx_buff[PandSFD_Index+15] = in_pBgk_oper->bgk_comm_buff.LinkSrcAddr[5];
                    
                    sdt_int16u i;
                    for(i = 0; i < in_pBgk_oper->bgk_comm_buff.PayloadLength;i++)
                    {
                        in_pBgk_oper->bgk_tx_buff[(PandSFD_Index+16)+i] = in_pBgk_oper->bgk_comm_buff.Payload[i];
                    }
                    Crc16CalculateOfByte(&in_pBgk_oper->bgk_tx_buff[PandSFD_Index+0],(Data_Length-2),&in_pBgk_oper->bgk_tx_buff[(PandSFD_Index+16)+in_pBgk_oper->bgk_comm_buff.PayloadLength]);
                    
                    in_pBgk_oper->transmit_len = PandSFD_Index+Data_Length;  //需要传送的字节数
                    in_pBgk_oper->transmit_index = 0;
                    pbc_reload_timerClock(&in_pBgk_oper->timer_tx_timeout,TIMEOUTV);
                    in_pBgk_oper->tx_feedback_index = 0;
                    if(in_pBgk_oper->pull_phy_busy())
                    {
                        in_pBgk_oper->transmit_monitor = bgk_trans_mon_conflict;
                        in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                        pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_tx_timeout);
                    }
                    else
                    {
                        in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                        in_pBgk_oper->entry_phy_tx();               //进入tx模式
                        
                        sdt_bool conflict_check;
                        if(bgk_bits_tx_conflict & in_pBgk_oper->bgk_bits)
                        {
                            conflict_check = sdt_true;
                        }
                        else
                        {
                            conflict_check = sdt_false;
                        }
                        
                        sdt_int16u remain_bytes;
                        
                        remain_bytes = in_pBgk_oper->transfet_bytes_to_phy_tx(&in_pBgk_oper->bgk_tx_buff[in_pBgk_oper->transmit_index],(in_pBgk_oper->transmit_len - in_pBgk_oper->transmit_index),conflict_check);
                        in_pBgk_oper->transmit_index = in_pBgk_oper->transmit_len - remain_bytes;
                        in_pBgk_oper->bgk_run_status = bgk_rs_tx_doing;
                    }
                }
                break;
            }
            case bgk_rs_tx_doing:
            {
                sdt_int16u remain_bytes;
                
                #ifdef TX_FEEDBACK_EN
                if(in_pBgk_oper->tx_feedback_index < in_pBgk_oper->transmit_len)
                {
                    if(in_pBgk_oper->bgk_bits & bgk_bits_rx_byte)
                    {
                        in_pBgk_oper->bgk_bits &= ~bgk_bits_rx_byte;
                        if(in_pBgk_oper->rx_byte == in_pBgk_oper->bgk_tx_buff[in_pBgk_oper->tx_feedback_index])
                        {
                            in_pBgk_oper->tx_feedback_index ++;
                        }
                        else
                        {
                            in_pBgk_oper->transmit_monitor = bgk_trans_mon_conflict;
                            in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                            pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_tx_timeout);
                        }
                    }                    
                }
                #else
                #endif
                if(in_pBgk_oper->transmit_index == in_pBgk_oper->transmit_len) //数据转移完毕,检测PHY发送完成
                {
                    if(in_pBgk_oper->pull_complete_tx_data())
                    {
                        #ifdef TX_FEEDBACK_EN

                        if(in_pBgk_oper->tx_feedback_index == in_pBgk_oper->transmit_len)
                        {
                            in_pBgk_oper->bgk_run_status = bgk_rs_tx_complete;
                        }
                        #else
                        in_pBgk_oper->bgk_run_status = bgk_rs_tx_complete;
                        #endif                        
                    }

                }
                else
                {
                    sdt_bool conflict_check;
                    if(bgk_bits_tx_conflict & in_pBgk_oper->bgk_bits)
                    {
                        conflict_check = sdt_true;
                    }
                    else
                    {
                        conflict_check = sdt_false;
                    }
                    remain_bytes = in_pBgk_oper->transfet_bytes_to_phy_tx(&in_pBgk_oper->bgk_tx_buff[in_pBgk_oper->transmit_index],(in_pBgk_oper->transmit_len - in_pBgk_oper->transmit_index),conflict_check);
                    in_pBgk_oper->transmit_index = in_pBgk_oper->transmit_len - remain_bytes;                    
                }
                break;
            }
            case bgk_rs_tx_complete:
            {
                in_pBgk_oper->transmit_monitor = bgk_trans_mon_complete;
                in_pBgk_oper->bgk_run_status = bgk_rs_idle;
                pbc_stop_timerIsOnceTriggered(&in_pBgk_oper->timer_tx_timeout);
                break;
            }
            default:
            {
                rd_bgk_run_status = bgk_rs_idle;
                break;
            }
        }
        
    }while(rd_bgk_run_status != in_pBgk_oper->bgk_run_status);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//半双工bough链路处理模块
//接收采用独立缓冲区方式
//发送数据共享链路数据区
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void alone_bough_link_task(bgk_oper_def* in_pBgk_oper)
{
    in_pBgk_oper->pull_look_for_byte_rx();
    bough_link_state_jump(in_pBgk_oper);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//soild 
#include ".\solid_bough_link_half.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++interface++++++++++++++++++++++++++++++++++++
//链路主任务
//------------------------------------------------------------------------------
void mde_bough_link_task(void)
{
    static sdt_bool cfged = sdt_false;

    if(cfged)
    {
        sdt_int8u i;
        for(i = 0;i < max_solid;i ++)
        {
            alone_bough_link_task(&bgk_oper_solid[i]);
        }
    }
    else
    {
        cfged = sdt_true;
        bgk_solid_cfg();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:获取链路忙碌状态
//链路忙碌时，对链路进行写入操作，会发生位置的错误
//in:  in_solid_number -- 实例编号
//out: sdt_true  -- link is busy. 
//     sdt_false -- link is idle.
//------------------------------------------------------------------------------
sdt_bool pull_bough_link_is_busy(sdt_int8u in_solid_number)
{
    sdt_bool the_busy = sdt_false;
    
    if(in_solid_number < max_solid)
    {
        if(bgk_oper_solid[in_solid_number].pull_phy_busy())  //phy busy
        {
            the_busy = sdt_true;
        }
        if(bgk_rs_idle == bgk_oper_solid[in_solid_number].bgk_run_status) 
        {
        }
        else if(bgk_rs_rx_preamble == bgk_oper_solid[in_solid_number].bgk_run_status) 
        {
        }
        else//link busy
        {
            the_busy = sdt_true;
        }
    }
    else
    {
        while(1);
    }
    return(the_busy);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:获取接收报文信息
//in:   in_solid_number -- 实例编号
//out:  sdt_true  -- 接收到一个完整的报文
//      sdt_false -- 没有接收到完整的报文
//------------------------------------------------------------------------------
sdt_bool pull_bough_recevie_one_message(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        if(bgk_bits_message_rx & bgk_oper_solid[in_solid_number].bgk_bits)
        {
            bgk_oper_solid[in_solid_number].bgk_bits &= ~bgk_bits_message_rx;
            bough_link_state_jump(&bgk_oper_solid[in_solid_number]);
            
            return(sdt_true);
        }
    }
    else
    {
        while(1);
    }
    return(sdt_false);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:获取链路缓冲区指针
//in:   in_solid_number -- 实例编号
//out:  缓冲区指针
//------------------------------------------------------------------------------
bgk_comm_buff_def* pull_bough_message_pBuff(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        return (&bgk_oper_solid[in_solid_number].bgk_comm_buff);
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:推入一字节的接收数据到链路
//in:   in_solid_number -- 实例编号
//      in_rx_byte      -- 接收的数据
//out:  none
//------------------------------------------------------------------------------
void push_bough_one_receive_byte(sdt_int8u in_solid_number,sdt_int8u in_rx_byte)
{
    if(in_solid_number < max_solid)
    {
        bgk_oper_solid[in_solid_number].bgk_bits |= bgk_bits_rx_byte;
        bgk_oper_solid[in_solid_number].rx_byte = in_rx_byte;
        bough_link_state_jump(&bgk_oper_solid[in_solid_number]);//数据进入后,马上处理状态
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:激活发送一个报文数据,先填充缓冲区数据，再激活发送
//in:   in_solid_number -- 实例编号
//      
//out:  none
//------------------------------------------------------------------------------
void push_active_one_message_transmit(sdt_int8u in_solid_number,sdt_bool conflict_check)
{
    if(in_solid_number < max_solid)
    {
        if(pull_bough_link_is_busy(in_solid_number))
        {
            bgk_oper_solid[in_solid_number].transmit_monitor = bgk_trans_mon_conflict;
        }
        else
        {
            bgk_oper_solid[in_solid_number].transmit_monitor = bgk_trans_mon_doing;
            bgk_oper_solid[in_solid_number].bgk_run_status = bgk_rs_tx_start;
            if(conflict_check)
            {
                bgk_oper_solid[in_solid_number].bgk_bits |= bgk_bits_tx_conflict;
            }
            else
            {
                bgk_oper_solid[in_solid_number].bgk_bits &= ~bgk_bits_tx_conflict;
            }
        }
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//name:数据发送状态监控
//in:  in_solid_number -- 实例编号
//out: bgk_trans_mon_none     -- 无意义
//     bgk_trans_mon_doing    -- 发送进行中
//     bgk_trans_mon_conflict -- PHY有冲突报文
//     bgk_trans_mon_complete -- 发送完成
//     bgk_trans_mon_error    -- 未知错误
//------------------------------------------------------------------------------
bgk_trans_mon_def pull_bough_transmit_monitor(sdt_int8u in_solid_number)
{
    if(in_solid_number < max_solid)
    {
        return(bgk_oper_solid[in_solid_number].transmit_monitor);
    }
    else
    {
        while(1);
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


