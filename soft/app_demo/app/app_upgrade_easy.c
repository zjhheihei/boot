//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//负责Bough Easy Upgrade Protocol的报文处理,任务。
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\APP_Cfg.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define BGPTCL_EasyUpagrde         0xD002  //简易固件升级协议
#define BGPTCL_debug               0xDFFF  //
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define BGEUCMD_QuerySalveStatus   0x01
#define BGEUCMD_EnterBoot          0x02
#define BGEUCMD_StrUpgrade_map     0x03
#define BGEUCMD_ResumeUpgrade_map  0x04
#define BGEUCMD_TransferFile       0x05

#define BGEUCMD_ErrorReport        0xFE
//-------------------------------------------------------------------------------------------------
#define BGEUERR_NoneError          0x00
#define BGEUERR_Length             0x01
#define BGEUERR_Version            0x02
#define BGEUERR_Command            0x03
#define BGEUERR_FileMapCheckSum    0x04
#define BGEUERR_FileCheckSum       0x05
#define BGEUERR_BlockNumber        0x06
#define BGEUERR_Fireware           0x07
#define BGEUERR_Other              0xFE
//--------------------------------------------------------------------------------------------------
#define BGEUSTS_None               0x00
#define BGEUSTS_NotInBoot          0x01
#define BGEUSTS_Boot               0x10
//#define BGEUSTS_QueryFileMap       0x11
#define BGEUSTS_QueryFile          0x12
#define BGEUSTS_UpgradeFinish      0x1F
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define THE_VER      0x01
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum
{
    ugd_esay_staue_idle         = 0x00,
    ugd_esay_staue_wait_ms      = 0x01,
    ugd_esay_staue_start_answer = 0x02,
    ugd_esay_staue_cpl_answer   = 0x03,
    ugd_esay_staue_backoff      = 0x04,
    ugd_esay_staue_backoffing   = 0x05,
}ugd_esay_staues_def;
//------------------------------------------------------------------------------
static ugd_esay_staues_def ugd_esay_staues;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct
{
   sdt_int8u  src_addr[6];
   sdt_int8u  dst_addr[6];
   sdt_int8u  answer_length;
   sdt_int8u  answer_payload[6];
}answer_oper_def;
//------------------------------------------------------------------------------
static answer_oper_def  answer_oper;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: Bough 错误应答
//功能: 
//入口: 
//      _Out_pPayload       应答内容,          ---->>
//      _In_ErrCode          错误号            <<----
//出口: 应答的数据长度,
//------------------------------------------------------------------------------
void Bough_AnswerErrorReport(sdt_int8u _In_ErrCode)
{
    answer_oper.answer_payload[0] = THE_VER;              //Version
    answer_oper.answer_payload[1] = BGEUCMD_ErrorReport;  //Command
    answer_oper.answer_payload[2] = BGEUSTS_None;         //Status
    answer_oper.answer_payload[3] = _In_ErrCode;          //Error Code
    answer_oper.answer_length = 4;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: Bough 简易升级报文协议处理函数
//功能: 
//入口: 
//      _In_pReceiveData     接收到的数据指针,         <<----
//     _In_TheAddr           本机链路地址              <<----
//      _Out_pTransmitData   应答数据指针,             ---->>
//      _Out_DevEnterBoot    进入Boot,sdt_true --需要进入   ---->>
//出口: 是否有应答数据,sdt_true--有应答数据
//-------------------------------------------------------------------------------------------------
void Bough_EasyUpgrade_Protocol(bgk_comm_buff_def* _In_pReceiveData,sdt_int8u* _In_TheAddr)
{         

    if(((0xFF == _In_pReceiveData->LinkDstAddr[0])&&\
        (0xFF == _In_pReceiveData->LinkDstAddr[1])&&\
        (0xFF == _In_pReceiveData->LinkDstAddr[2])&&\
        (0xFF == _In_pReceiveData->LinkDstAddr[3])&&\
        (0xFF == _In_pReceiveData->LinkDstAddr[4])&&\
        (0xFF == _In_pReceiveData->LinkDstAddr[5]))||\
       ((_In_TheAddr[0] == _In_pReceiveData->LinkDstAddr[0])&&\
        (_In_TheAddr[1] == _In_pReceiveData->LinkDstAddr[1])&&\
        (_In_TheAddr[2] == _In_pReceiveData->LinkDstAddr[2])&&\
        (_In_TheAddr[3] == _In_pReceiveData->LinkDstAddr[3])&&\
        (_In_TheAddr[4] == _In_pReceiveData->LinkDstAddr[4])&&\
        (_In_TheAddr[5] == _In_pReceiveData->LinkDstAddr[5])))
    {

        answer_oper.dst_addr[0] = _In_pReceiveData->LinkSrcAddr[0];
        answer_oper.dst_addr[1] = _In_pReceiveData->LinkSrcAddr[1];
        answer_oper.dst_addr[2] = _In_pReceiveData->LinkSrcAddr[2];
        answer_oper.dst_addr[3] = _In_pReceiveData->LinkSrcAddr[3];
        answer_oper.dst_addr[4] = _In_pReceiveData->LinkSrcAddr[4];
        answer_oper.dst_addr[5] = _In_pReceiveData->LinkSrcAddr[5];
        
        answer_oper.src_addr[0] = _In_TheAddr[0];
        answer_oper.src_addr[1] = _In_TheAddr[1];
        answer_oper.src_addr[2] = _In_TheAddr[2];
        answer_oper.src_addr[3] = _In_TheAddr[3];
        answer_oper.src_addr[4] = _In_TheAddr[4];
        answer_oper.src_addr[5] = _In_TheAddr[5];
        
        if(0 == _In_pReceiveData->PayloadLength)
        {
            Bough_AnswerErrorReport(BGEUERR_Length);
        }
        else
        {
            if(THE_VER == _In_pReceiveData->Payload[0])  //Command
            {
                switch(_In_pReceiveData->Payload[1])
                {
                    case BGEUCMD_QuerySalveStatus://Query Boot Status
                    {
                        if(4 == _In_pReceiveData->PayloadLength)
                        {
                            answer_oper.answer_payload[0] = THE_VER;                           //Version
                            answer_oper.answer_payload[1] = (BGEUCMD_QuerySalveStatus | 0x80); //Command
                            answer_oper.answer_payload[2] = BGEUSTS_Boot;                      //Status
                            answer_oper.answer_payload[3] = BGEUERR_NoneError;                 //Error Code
                            answer_oper.answer_length = 4;                               
                        }
                        else
                        {
                            Bough_AnswerErrorReport(BGEUERR_Length);
                        }
                        break;
                    }
                    case BGEUCMD_EnterBoot:
                    {
                        if(4 == _In_pReceiveData->PayloadLength)
                        {
                            answer_oper.answer_payload[0] = THE_VER;                           //Version
                            answer_oper.answer_payload[1] = (BGEUCMD_EnterBoot | 0x80);        //Command
                            answer_oper.answer_payload[2] = BGEUSTS_Boot;     //Status
                            answer_oper.answer_payload[3] = BGEUERR_NoneError;                 //Error Code
                            answer_oper.answer_length = 4;                          
                        }
                        else
                        {
                            Bough_AnswerErrorReport(BGEUERR_Length);
                        }
                        
                        break;
                    }
                    case BGEUCMD_StrUpgrade_map://重传命令
                    {
                        
                        if((128+6) == _In_pReceiveData->PayloadLength)
                        {
                            sdt_int8u err;
                            err = mde_push_fileMap(&_In_pReceiveData->Payload[6],sdt_false);
                            if(err)
                            {
                                Bough_AnswerErrorReport(err);
                            }
                            else
                            {
                                answer_oper.answer_payload[0] = THE_VER;                                //Version
                                answer_oper.answer_payload[1] = (BGEUCMD_StrUpgrade_map | 0x80);  //Command
                                answer_oper.answer_payload[2] = BGEUSTS_QueryFile;                   //Status,请求FileMap
                                answer_oper.answer_payload[3] = BGEUERR_NoneError;                                     //Error Code
                                
                                sdt_int16u next_block_number;
                                mde_pull_upgrade_next_block(&next_block_number);
                                pbc_int16uToArray_bigEndian(next_block_number,&answer_oper.answer_payload[4]);
                                answer_oper.answer_length = 6;                                   
                            }
                        }
                        else
                        {
                            Bough_AnswerErrorReport(BGEUERR_Length);
                        }
                        break;
                    }
                    case BGEUCMD_ResumeUpgrade_map://续传命令
                    {
                      
                        if((128+6) == _In_pReceiveData->PayloadLength)
                        {
                            sdt_int8u err;
                            err = mde_push_fileMap(&_In_pReceiveData->Payload[6],sdt_true);
                            if(err)
                            {
                                Bough_AnswerErrorReport(err);
                            }
                            else
                            {
                                answer_oper.answer_payload[0] = THE_VER;                              //Version
                                answer_oper.answer_payload[1] = (BGEUCMD_ResumeUpgrade_map | 0x80);        //Command
                                answer_oper.answer_payload[2] = BGEUSTS_QueryFile;                  //Status,请求FileMap
                                answer_oper.answer_payload[3] = BGEUERR_NoneError;                    //Error Code
                                
                                sdt_int16u next_block_number;
                                mde_pull_upgrade_next_block(&next_block_number);
                                pbc_int16uToArray_bigEndian(next_block_number,&answer_oper.answer_payload[4]);
                                answer_oper.answer_length = 6;                                     
                            }
                        }
                        else
                        {
                            Bough_AnswerErrorReport(BGEUERR_Length);
                        }
                        break;
                    }
                    case BGEUCMD_TransferFile://接收到File内容
                    {
                        if((128+6) == _In_pReceiveData->PayloadLength)
                        {
                            sdt_int8u ErrorCode = BGEUERR_NoneError;

                            ErrorCode = mde_push_files_one_block(pbc_arrayToInt16u_bigEndian(&_In_pReceiveData->Payload[4]),&_In_pReceiveData->Payload[6]);  //处理接收报文
                            
                            sdt_bool completed;
                            sdt_int16u next_block_number;
                            
                            completed = mde_pull_upgrade_next_block(&next_block_number);
                            if(ErrorCode)
                            {
                                Bough_AnswerErrorReport(ErrorCode);
                            }
                            else if(completed)
                            {
                                answer_oper.answer_payload[0] = THE_VER;                              //Version
                                answer_oper.answer_payload[1] = (BGEUCMD_TransferFile | 0x80);         //Command
                                answer_oper.answer_payload[2] = BGEUSTS_UpgradeFinish;                 //Status,UpgradeFinish
                                answer_oper.answer_payload[3] = BGEUERR_NoneError;                     //Error Code
                                answer_oper.answer_payload[4] = 0;
                                answer_oper.answer_payload[5] = 0;
                                answer_oper.answer_length = 6;   
                            }
                            else
                            {
                                answer_oper.answer_payload[0] = THE_VER;                           //Version
                                answer_oper.answer_payload[1] = (BGEUCMD_TransferFile | 0x80);     //Command
                                answer_oper.answer_payload[2] = BGEUSTS_QueryFile;                 //Status,请求File
                                answer_oper.answer_payload[3] = BGEUERR_NoneError;                 //Error Is not
                                pbc_int16uToArray_bigEndian(next_block_number,&answer_oper.answer_payload[4]);
                                answer_oper.answer_length = 6;    
                            }
                        }
                        else
                        {
                            Bough_AnswerErrorReport(BGEUERR_Length);
                        }
                        break;
                    }
                    default:
                    {
                        Bough_AnswerErrorReport(BGEUCMD_ErrorReport);
                        break;
                    }
                }
            }
            else
            {
                Bough_AnswerErrorReport(BGEUERR_Version);
            }          
        }
        ugd_esay_staues = ugd_esay_staue_start_answer;
        app_upgrade_easy_task();  //运行一次任务,有发送数据,占领link
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//升级接收文件状态

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//名称: 获取设备是否运行在Boot状态
//功能: 
//入口: 
//      
//出口: sdt_true--Boot状态
//-------------------------------------------------------------------------------------------------
sdt_int8u Pull_TheDeviceUpgradeStatus(void)
{
    sdt_int8u TheUdeStatus = BGEUSTS_Boot;
    /*
    switch(UPL_RevFileStatus)
    {
        case UPL_Idle:
        {
            break;
        }
        case UPL_RevFileMap:
        {
            TheUdeStatus = BGEUSTS_QueryFileMap;
            break;
        }
        case UPL_RevFileData:
        {
            TheUdeStatus = BGEUSTS_QueryFile;
            break;
        }
        case UPL_RevFinish:
        {
            TheUdeStatus = BGEUSTS_UpgradeFinish;
            break;
        }
        default:
        {
            break;
        }
    }*/
    return(TheUdeStatus);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//重传处理
//-------------------------------------------------------------------------------------------------
void RestartUpgrade_Process(void)
{

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//续传处理
//-------------------------------------------------------------------------------------------------
void ResumeUpgrade_Process(void)
{

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
sdt_int16u random_backoff_time(void)
{
    return(pull_mde_one_random_data());
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//简易升级任务
//------------------------------------------------------------------------------
void app_upgrade_easy_task(void)
{
    mde_upgrade_files_task();
    macro_createTimer(timer_backoff,timerType_millisecond,0);
    ugd_esay_staues_def rd_ugd_esay_staues;
    
    
    pbc_timerClockRun_task(&timer_backoff);

    do
    {
        rd_ugd_esay_staues = ugd_esay_staues;
        switch(ugd_esay_staues)
        {
            case ugd_esay_staue_idle:
            {
                ugd_esay_staues = ugd_esay_staue_wait_ms;
                break;
            }
            case ugd_esay_staue_wait_ms:
            {
                break;
            }
            case ugd_esay_staue_start_answer:
            {
                if(pull_bough_link_is_busy(0))
                {
                    ugd_esay_staues = ugd_esay_staue_backoff;
                }
                else
                {
                  
                    bgk_comm_buff_def* p_tx_bgk_buff;
                    
                    p_tx_bgk_buff = pull_bough_message_pBuff(0);
                    p_tx_bgk_buff->LinkSrcAddr[0] = answer_oper.src_addr[0];
                    p_tx_bgk_buff->LinkSrcAddr[1] = answer_oper.src_addr[1];
                    p_tx_bgk_buff->LinkSrcAddr[2] = answer_oper.src_addr[2];
                    p_tx_bgk_buff->LinkSrcAddr[3] = answer_oper.src_addr[3];
                    p_tx_bgk_buff->LinkSrcAddr[4] = answer_oper.src_addr[4];
                    p_tx_bgk_buff->LinkSrcAddr[5] = answer_oper.src_addr[5];
                    
                    p_tx_bgk_buff->LinkDstAddr[0] = answer_oper.dst_addr[0];
                    p_tx_bgk_buff->LinkDstAddr[1] = answer_oper.dst_addr[1];
                    p_tx_bgk_buff->LinkDstAddr[2] = answer_oper.dst_addr[2];
                    p_tx_bgk_buff->LinkDstAddr[3] = answer_oper.dst_addr[3];
                    p_tx_bgk_buff->LinkDstAddr[4] = answer_oper.dst_addr[4];
                    p_tx_bgk_buff->LinkDstAddr[5] = answer_oper.dst_addr[5];     
                    p_tx_bgk_buff->ProcotolType = BGPTCL_EasyUpagrde;
                    p_tx_bgk_buff->PayloadLength = answer_oper.answer_length;
                    
                    sdt_int8u i;
                    for(i = 0;i<answer_oper.answer_length;i++)
                    {
                        p_tx_bgk_buff->Payload[i] = answer_oper.answer_payload[i];
                    }
                    push_active_one_message_transmit(0,sdt_false);
                    ugd_esay_staues = ugd_esay_staue_cpl_answer;
                }
                break;
            }
            case ugd_esay_staue_cpl_answer:
            {
                bgk_trans_mon_def rd_monitor;
                rd_monitor = pull_bough_transmit_monitor(0);
                if(bgk_trans_mon_conflict == rd_monitor)
                {
                    ugd_esay_staues = ugd_esay_staue_backoff;
                }
                else if(bgk_trans_mon_complete == rd_monitor)
                {
                    ugd_esay_staues = ugd_esay_staue_wait_ms;
                }
                else if(bgk_trans_mon_error == rd_monitor)
                {
                    ugd_esay_staues = ugd_esay_staue_wait_ms;
                }
                break;
            }
            case ugd_esay_staue_backoff:
            {
                pbc_reload_timerClock(&timer_backoff,random_backoff_time());//随机避退时间
                ugd_esay_staues = ugd_esay_staue_backoffing;
                break;
            }
            case ugd_esay_staue_backoffing:
            {
                if(pbc_pull_timerIsCompleted(&timer_backoff))
                {
                    ugd_esay_staues = ugd_esay_staue_start_answer;
                }
                break;
            }
            default:
            {
                ugd_esay_staues = ugd_esay_staue_idle;
                break;
            }
        }
    }while(rd_ugd_esay_staues != ugd_esay_staues);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//------------------------------------------------------------------------------
  
//------------------------------------------------------------------------------

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++