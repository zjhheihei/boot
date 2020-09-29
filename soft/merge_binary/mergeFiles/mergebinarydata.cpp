#include "mergebinarydata.h"
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
mergeBinaryData::mergeBinaryData()
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define __int32u unsigned int
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define CheckSumWordEven          0x39ea2e76
#define CheckSumWordOdd           0x82b453c3
#define VersionTuf                0x00000001
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
__int32u MakeOneCheckText(__int32u Data,__int32u Count)
{
    if(Count&0x00000001)
    {
        Data=Data^CheckSumWordOdd;
    }
    else
    {
        Data=Data^CheckSumWordEven;
    }

    return(Data);
}
//-------------------------------------------------------------------------------
__int32u MakeCheckSumText(char* pData,__int32u Length)
{
    __int32u iWords,This_ids;
    __int32u Index;
    __int32u MakeSum,ReadData;
    iWords = (Length)/4;
    This_ids = 0;
    MakeSum = 0;
    Index = 0;
    while(iWords)
    {
        ReadData = (__int32u)pData[Index]&0x000000ff;
        ReadData = ReadData<<8;
        ReadData = ReadData|((__int32u)pData[Index+1]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((__int32u)pData[Index+2]&0x000000ff);
        ReadData = ReadData<<8;
        ReadData = ReadData|((__int32u)pData[Index+3]&0x000000ff);
        MakeSum+=MakeOneCheckText(ReadData,This_ids);
        Index = Index+4;
        iWords--;
        This_ids++;
    }
    return(MakeSum);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//出口false 合并失败
//----------------------------------------------------------------------------------------
bool mergeBinaryData::mergeBinary_createMerge(binaryTransfer_def in_binaryOrigin,QByteArray* out_mergeBin)
{
    #define BootAreaMaxBytes       0x0800
    #define AppAreaMaxBytes        0x7C00

    #define UPDATA_APPALLOWRUN    0x519AE493

    #define Sto_UpdataSign3       AppArray[AppAreaMaxBytes-12]
    #define Sto_UpdataSign2       AppArray[AppAreaMaxBytes-11]
    #define Sto_UpdataSign1       AppArray[AppAreaMaxBytes-10]
    #define Sto_UpdataSign0       AppArray[AppAreaMaxBytes-9]
    #define Sto_AppCheckSum3      AppArray[AppAreaMaxBytes-8]
    #define Sto_AppCheckSum2      AppArray[AppAreaMaxBytes-7]
    #define Sto_AppCheckSum1      AppArray[AppAreaMaxBytes-6]
    #define Sto_AppCheckSum0      AppArray[AppAreaMaxBytes-5]
    #define Sto_BytesOfAppRun3    AppArray[AppAreaMaxBytes-4]
    #define Sto_BytesOfAppRun2    AppArray[AppAreaMaxBytes-3]
    #define Sto_BytesOfAppRun1    AppArray[AppAreaMaxBytes-2]
    #define Sto_BytesOfAppRun0    AppArray[AppAreaMaxBytes-1]

    unsigned int i;
    unsigned char BootArray[BootAreaMaxBytes];
    unsigned char AppArray[AppAreaMaxBytes];
    bool mergeRight = true;

    binaryTransfer_def read_binaryOrigin;

    read_binaryOrigin = in_binaryOrigin;


    unsigned int appBinaryLength;
    appBinaryLength = read_binaryOrigin.in_appArea_length + (128-(read_binaryOrigin.in_appArea_length)%128); //aligning to 128byts

    if((read_binaryOrigin.in_bootArea_length > BootAreaMaxBytes) || (read_binaryOrigin.in_bootArea_length > AppAreaMaxBytes))
    {
        mergeRight = false;
    }
    else
    {

        for(i = 0;i < BootAreaMaxBytes;i++)
        {
            if(i < read_binaryOrigin.in_bootArea_length)
            {
                BootArray[i] = read_binaryOrigin.in_pBootArea_data[i];
            }
            else
            {
                BootArray[i] = 0x00;
            }
        }
        //boot区中断向量重定义
        for(i = 0;i<31;i++)
        {
            BootArray[i*4+4] = 0x82;
            BootArray[i*4+5] = 0x00;
            BootArray[i*4+6] = 0x88;
            BootArray[i*4+7] = 0x04+i*4;
        }
        for(i = 0;i < AppAreaMaxBytes;i++)
        {
            if(i < read_binaryOrigin.in_appArea_length)
            {
                AppArray[i] = read_binaryOrigin.in_pAppArea_data[i];
            }
            else if(i < appBinaryLength)
            {
                AppArray[i] = 0xff;
            }
            else
            {
                AppArray[i] = 0x00;
            }
        }
        __int32u CheckSum;
        CheckSum = MakeCheckSumText((char*)&AppArray[0],appBinaryLength);
        Sto_AppCheckSum3 = CheckSum >>24;
        Sto_AppCheckSum2 = CheckSum >>16;
        Sto_AppCheckSum1 = CheckSum >>8;
        Sto_AppCheckSum0 = CheckSum;

        Sto_BytesOfAppRun3 = appBinaryLength>>24;
        Sto_BytesOfAppRun2 = appBinaryLength>>16;
        Sto_BytesOfAppRun1 = appBinaryLength>>8;
        Sto_BytesOfAppRun0 = appBinaryLength;

        Sto_UpdataSign3 = (unsigned char)(UPDATA_APPALLOWRUN>>24);
        Sto_UpdataSign2 = (unsigned char)(UPDATA_APPALLOWRUN>>16);
        Sto_UpdataSign1 = (unsigned char)(UPDATA_APPALLOWRUN>>8);
        Sto_UpdataSign0 = (unsigned char)(UPDATA_APPALLOWRUN);

        out_mergeBin->clear();

        for(i=0;i<sizeof(BootArray);i++)
        {
            out_mergeBin->append(BootArray[i]);
        }
        for(i=0;i<sizeof(AppArray);i++)
        {
           out_mergeBin->append(AppArray[i]);
        }
        for(i=0;i<sizeof(AppArray);i++)  //update area data
        {
           out_mergeBin->append(AppArray[i]);
        }
    }
    return(mergeRight);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
