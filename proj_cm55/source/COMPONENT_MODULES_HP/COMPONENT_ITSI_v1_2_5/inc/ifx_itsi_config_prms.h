/*****************************************************************************
ifx_itsi_config+prms.h: ITSI default parameter configuration header file.

Copyright 2024 Infineon Technologies.  ALL RIGHTS RESERVED.
******************************************************************************/
#ifndef __ITSI_CONFIG_PRMS_H
#define __ITSI_CONFIG_PRMS_H

#include "ifx_pre_post_process.h"
int32_t sod_prms[] = {
    0,     /* manunally generated configuration file set configuration version to zero */
    16000, /* sampling rate */
    160,   /* input frmae size */
    IFX_PRE_PROCESS_IP_COMPONENT_SOD,     /* IP_compnent_id: SOD */
    2,     /* number of parameters */
    400,   /* gap setting */
    16384  /* sensitivity */
};

int32_t pre_proc_hpf_prms[] = {
    0,     /* manunally generated configuration file set configuration version to zero */
    16000, /* sampling rate */
    160,   /* input frmae size */
    IFX_PRE_PROCESS_IP_COMPONENT_HPF,     /* IP_compnent_id: preprocess HPF */
    0,     /* number of parameters */
};

int32_t denoise_prms[] = {
      0,     /* manunally generated configuration file set configuration version to zero */
      16000,   /* sampling rate */
      160,   /* audio frmae size in samples */
      IFX_PRE_PROCESS_IP_COMPONENT_DENOISE,     /* IP_compnent_id: DENOISE feature */
      1,     /* number of parameters in DENOISE feature */
      38,   /* DENOISE feature output size */
    };

int32_t dfww_prms[] = {
  0,     /* manunally generated configuration file set configuration version to zero */
  16000,   /* sampling rate */
  160,   /* audio frmae size in samples */
  IFX_POST_PROCESS_IP_COMPONENT_DFWWD,     /* IP_compnent_id: DFWWD feature */
  1,     /* number of parameters in DFWWD feature */
  500,   /* noactivty_timeout between SOD and WW in ms */
};

int32_t dfcmd_prms[] = {
  0,     /* manunally generated configuration file set configuration version to zero */
  16000,   /* sampling rate */
  160,   /* audio frmae size in samples */
  IFX_POST_PROCESS_IP_COMPONENT_DFCMD,     /* IP_compnent_id: DFCCMD feature */
  1,     /* number of parameters in DFCMD feature */
  2000,   /* maxwwgap between the end of the WW and the beginning of the commands in ms */
};

#define DFWW_PARM_INDEX    5
#define DFCMD_PARM_INDEX   5

#endif /* __ITSI_CONFIG_PRMS_H */
