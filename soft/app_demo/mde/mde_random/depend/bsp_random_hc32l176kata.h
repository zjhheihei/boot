//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include ".\snail_data_types.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "hc32l17x.h"
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_random_cfg(void)
{
    M0P_SYSCTRL->PERI_CLKEN0_f.RNG = 1;
    
    M0P_TRNG->CR_f.RNGCIR_EN = 1;
    M0P_TRNG->MODE_f.LOAD = 1;
    M0P_TRNG->MODE_f.FDBK = 1;
    M0P_TRNG->MODE_f.CNT = 6;
    M0P_TRNG->CR_f.RNG_RUN = 1;
    while(0 != M0P_TRNG->CR_f.RNG_RUN)
    {
    }
    M0P_TRNG->MODE_f.LOAD = 0;
    M0P_TRNG->MODE_f.FDBK = 0;
    M0P_TRNG->MODE_f.CNT = 4;
    M0P_TRNG->CR_f.RNG_RUN = 1;
    while(0 != M0P_TRNG->CR_f.RNG_RUN)
    {
    }
    M0P_TRNG->CR_f.RNGCIR_EN = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static enum
{
   trng_rs_idle     = 0x00,
   trng_rs_tgs0     = 0x01,
   trng_rs_tgs1     = 0x02,
   trng_rs_tgs2     = 0x03,
   trng_rs_cmp      = 0x04,
}trng_run_s = trng_rs_idle;
//------------------------------------------------------------------------------
static sdt_int16u random_data;
//------------------------------------------------------------------------------
void bsp_trng_task(void)
{
    switch(trng_run_s)
    {
        case trng_rs_idle:
        {
            
            break;
        }
        case trng_rs_tgs0:
        {
            M0P_TRNG->CR_f.RNGCIR_EN = 1;
            M0P_TRNG->MODE_f.LOAD = 0;
            M0P_TRNG->MODE_f.FDBK = 1;
            M0P_TRNG->MODE_f.CNT = 6;
            M0P_TRNG->CR_f.RNG_RUN = 1;
            trng_run_s = trng_rs_tgs1;
            break;
        }
        case trng_rs_tgs1:
        {
            if(0 == M0P_TRNG->CR_f.RNG_RUN)
            {
                M0P_TRNG->MODE_f.FDBK = 0;
                M0P_TRNG->MODE_f.CNT = 4;
                M0P_TRNG->CR_f.RNG_RUN = 1;
                trng_run_s = trng_rs_tgs2;
            }
            break;
        }
        case trng_rs_tgs2:
        {
            if(0 == M0P_TRNG->CR_f.RNG_RUN)
            {
                union
                {
                    sdt_int32u rd_rmd_32bits[2];
                    sdt_int64u rd_rmd_64bits;
                }rd_rmd;
                
                rd_rmd.rd_rmd_32bits[0] = M0P_TRNG->DATA0;

                rd_rmd.rd_rmd_32bits[1] = M0P_TRNG->DATA1;
                random_data = rd_rmd.rd_rmd_64bits%1000;
                while(random_data > 1000)
                {
                    random_data -= 1000;
                }
                while(random_data < 20)
                {
                    random_data += 20;
                }
                trng_run_s = trng_rs_cmp;
            }
            break;
        }
        case trng_rs_cmp:
        {
            break;
        }
        default:
        {
            trng_run_s = trng_rs_idle;
            break;
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void bsp_trng_run_once(void)
{
    trng_run_s = trng_rs_tgs0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
sdt_bool bps_pull_trng_complete(void)
{
    if(trng_rs_cmp == trng_run_s)
    {
        trng_run_s = trng_rs_idle;
        return(sdt_true);
    }
    else
    {
        return(sdt_false);
    }
}
//------------------------------------------------------------------------------
//控制在20-2000之间
//------------------------------------------------------------------------------
sdt_int16u bsp_pull_random_data(void)
{
    return(random_data);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
