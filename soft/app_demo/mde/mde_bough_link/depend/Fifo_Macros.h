//*********************************************************
#ifndef _FIFO_MACROS_H
#define _FIFO_MACROS_H
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//FIFO结构宏定义
//---------------------------------------------------------
typedef struct{
                  volatile sdt_int8u     fifo_inptr;
                  volatile sdt_int8u     fifo_outptr;
                  volatile sdt_int8u     fifo_busybyte;
              }fifo_queueDef;
//---------------------------------------------------------
//创建fifo宏
#define _Creat_my_fifo(fifo_name,fifo_size) \
        volatile sdt_int8u   fifo_name##_buff[fifo_size];\
        fifo_queueDef fifo_name##_queue;\
        const sdt_int8u fifo_name##_width=fifo_size
//---------------------------------------------------------
//声明fifo宏
#define _Declare_my_fifo(fifo_name,fifo_size) \
        extern volatile sdt_int8u     fifo_name##_buff[fifo_size];\
        extern fifo_queueDef fifo_name##_queue;\
        extern const sdt_int8u fifo_name##_width
//---------------------------------------------------------
//写入fifo一个数据
#define _In_my_fifo(fifo_name,data) \
        do{\
              if(fifo_name##_queue.fifo_busybyte < fifo_name##_width)\
              {\
                  fifo_name##_buff[fifo_name##_queue.fifo_inptr]=data;\
                  fifo_name##_queue.fifo_inptr++;\
                  if(fifo_name##_queue.fifo_inptr>(fifo_name##_width-1))\
                  {\
                      fifo_name##_queue.fifo_inptr=0;\
                  }\
                  fifo_name##_queue.fifo_busybyte++;\
              }\
          }while(0)
//---------------------------------------------------------
//读取fifo一个数据
#define _Out_my_fifo(fifo_name,data) \
        do{\
              if(fifo_name##_queue.fifo_busybyte!=0)\
              {\
                  data=fifo_name##_buff[fifo_name##_queue.fifo_outptr];\
                  fifo_name##_queue.fifo_busybyte--;\
                  fifo_name##_queue.fifo_outptr++;\
                  if(fifo_name##_queue.fifo_outptr>(fifo_name##_width-1))\
                  {\
                      fifo_name##_queue.fifo_outptr=0;\
                  }\
              }\
          }while(0)
//---------------------------------------------------------
//获取fifo有效字节数
#define _Get_my_fifo_byte(fifo_name,_nbyte) \
        do{\
              _nbyte=fifo_name##_queue.fifo_busybyte;\
          }while(0)
//---------------------------------------------------------
//获取fifo状态,【正常，空，满】
#define FIFO_NBER      0
#define FIFO_EMPTY     1
#define FIFO_FULL      2
#define _Get_my_fifo_state(fifo_name,_state)\
        do{\
              if(fifo_name##_queue.fifo_busybyte==0)\
              {\
                  _state=FIFO_EMPTY;\
              }\
              else if(fifo_name##_queue.fifo_busybyte==fifo_name##_width)\
              {\
                 _state=FIFO_FULL;\
              }\
              else\
              {\
                  _state=FIFO_NBER;\
              }\
          }while(0)
//---------------------------------------------------------
//初始化fifo
#define _Init_my_fifo(fifo_name)\
        do{\
             fifo_name##_queue.fifo_inptr=0;\
             fifo_name##_queue.fifo_outptr=0;\
             fifo_name##_queue.fifo_busybyte=0;\
          }while(0)
//---------------------------------------------------------
//*********************************************************
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++