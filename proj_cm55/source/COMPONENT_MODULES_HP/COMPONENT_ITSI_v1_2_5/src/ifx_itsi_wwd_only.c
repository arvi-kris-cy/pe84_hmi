/******************************************************************************
* File Name: ifx_wwd_only.c
*
* Description: This file contains functions to initialize and run frame by frame
*              Infineon ITSI WWD only process functions.
*
*
*******************************************************************************
* (c) 2024, Infineon Technologies Company. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Infineon Technologies Company (Infineon) or one of its
* subsidiaries and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Infineon hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Infineon's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Infineon.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Infineon
* reserves the right to make changes to the Software without notice. Infineon
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Infineon does
* not authorize its products for use in any products where a malfunction or
* failure of the Infineon product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Infineon's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Infineon against all liability.
*******************************************************************************/


/*******************************************************************************
* Include header file
******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ifx_pre_post_process.h"
#include "ifx_sp_common_priv.h"
#include "ifx_sp_utils.h"
#include "ifx_am_predict.h"
#include "ifx_itsi_config_prms.h"
#include "ifx_itsi_prms.h"
#include "ifx_itsi.h"

//////////////////////// MTB ML ////////////////////////

#if !defined(_MSC_VER) && defined(USE_MTB_ML)
#include "mtb_ml.h"
#undef MODEL_NAME
#define MODEL_NAME  ITSI_AM_LSTM
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Model information */
static mtb_ml_model_t* itsi_model_obj;

/* Following itsi_tensor_arena has been counted as part of persistent memory total szie */
/* Tensor_arena buffer must be in SOCMEM and aligned by 16 which are required by U55 */
static uint8_t itsi_tensor_arena[ITSI_AM_LSTM_ARENA_SIZE] __attribute__((aligned(16)))
                                          __attribute__((section(".cy_socmem_data")));

static mtb_ml_model_buffer_t itsi_model_buffer =
{
    .tensor_arena = itsi_tensor_arena,
    .tensor_arena_size = ITSI_AM_LSTM_ARENA_SIZE
};

static MTB_ML_DATA_T data_feed_int[N_SEQ * FEATURE_BUF_SZ] __attribute__((aligned(16)));

static float mtb_ml_input_buffer[N_SEQ * FEATURE_BUF_SZ];
#endif

//////////////////////// ITSI Data Container Pointers ////////////////////////
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
void* pre_proc_hpf_obj = NULL;
#endif
void* sod_obj = NULL;
void* feature_obj = NULL;
void* dfww_obj = NULL;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
void* am_obj = NULL;
#endif

#ifdef EMBEDDED_NO_FILE
#undef SAVE_FEATURES
#undef SAVE_LL_SCORES_TO_FILE
#else
typedef char modelmem_t;
modelmem_t WWmodeldata[MAX_WW_MODEL_SIZE];
#endif // EMBEDDED_NO_FILE

#if defined(AM_LOOKBACK)
/* To avoid compiler warning, AM_LOOKBACK is redefined below because it is coverted from float number. Please make sure this
   redefination deos not change its value. */
#undef AM_LOOKBACK
#define AM_LOOKBACK 5
#else
#define AM_LOOKBACK 5
#endif
#ifdef _MSC_VER
float xIn[FRAME_SIZE_16K];
float features[FEATURE_BUF_SZ];
float output_scores[(N_PHONEMES +1) * AM_LOOKBACK];
#else
float xIn[FRAME_SIZE_16K] __attribute__((section(".itsi_socmem_data3")));
float features[FEATURE_BUF_SZ] __attribute__((section(".itsi_socmem_data4")));
float output_scores[(N_PHONEMES +1) * (1 + AM_LOOKBACK)] __attribute__((section(".itsi_socmem_data5")));
#endif

ifx_scratch_mem_t itsi_scratch;   //ITSI scratch memory
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
#ifdef _MSC_VER
float am_buffer[AM_OUT_SZ_FOR_BUFFER * N_SEQ];
float am_buffer_copy[AM_OUT_SZ_FOR_BUFFER * N_SEQ];
#else
float am_buffer[AM_OUT_SZ_FOR_BUFFER * N_SEQ] __attribute__((section(".cy_itcm")));
float am_buffer_copy[AM_OUT_SZ_FOR_BUFFER * N_SEQ] __attribute__((section(".cy_itcm")));
#endif
#endif

int sodtrigcnt;
int32_t WWDresult; // -2 = init (or reset), -1 = no (i.e. rejected), 1 = yes (i.e. detected), 0 = indecision
int32_t feature_buf_cnt;

// for debugging
#ifdef ITSI_DEBUG
int frame_cnt;
int SODcnt;
#endif

#ifdef SAVE_FEATURES
extern FILE* features_bin_file;
extern int num_features_hops;
#endif

#if !defined(_MSC_VER) && defined(USE_MTB_ML)
#define NPU_PRIORITY 3
/*******************************************************************************
* Function Name: mtb_ml_inference_init
********************************************************************************
* Summary:
*  Initializes the neural network.
*
* Parameters:
*  None
*
* Returns:
*  The status of the initialization.
*
*******************************************************************************/
cy_rslt_t mtb_ml_inference_init(void)
{
    cy_rslt_t result;

    mtb_ml_model_bin_t itsi_model_bin = { MTB_ML_MODEL_BIN_DATA(MODEL_NAME) };

    /* Initialize the Neural Network */
    result = mtb_ml_model_init(&itsi_model_bin, &itsi_model_buffer, &itsi_model_obj);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }

    /* Set the priority of NPU interrupt handler */
    mtb_ml_init(NPU_PRIORITY);

    return result;
}
#endif

#ifndef EMBEDDED_NO_FILE
int read_itsi_ww_models(char* itsi_ww_model_file_path)
{
    if (itsi_ww_model_file_path != NULL)
    {
        FILE* fmodel = fopen(itsi_ww_model_file_path, "rb");
        if (fmodel == NULL)
        {
            printf("Could not read %s ITSI WW model.\n", itsi_ww_model_file_path);
            return -1;
        }
        fread((modelmem_t*)WWmodeldata, sizeof(char), MAX_WW_MODEL_SIZE, fmodel);
        fclose(fmodel);
    }
    return 0;
}
#endif

void ifx_reset_feature_buf(void)
{
#if !defined(_MSC_VER) && defined(USE_MTB_ML)
    memset(mtb_ml_input_buffer, 0, sizeof(float) * (FEATURE_BUF_SZ * N_SEQ));
#else
    memset(am_buffer, 0, sizeof(float) * (AM_OUT_SZ_FOR_BUFFER * N_SEQ));
    memset(am_buffer_copy, 0, sizeof(float) * (AM_OUT_SZ_FOR_BUFFER * N_SEQ));
#endif
}

#ifdef EMBEDDED_NO_FILE
uint32_t ifx_init_itsi_wwd(const char* ww_model_ptr)
#else
uint32_t ifx_init_itsi_wwd(char* itsi_am_prms_file_path, char* itsi_am_wts_file_path, char* its_am_bias_file_path, char* itsi_ww_model_file_path)
#endif
{
    int ErrorHdl = 0;
    ifx_stc_pre_post_process_info_t pre_proc_hpf_info;
    ifx_stc_pre_post_process_info_t sod_info;
    ifx_stc_pre_post_process_info_t feature_info;
    ifx_stc_pre_post_process_info_t dfww_info;
    int32_t scrtach_sz, am_predict_sz, persistent_mem_size = 0;
    (void)persistent_mem_size;
    /* Step 1: Parse and get required memory from configurations */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    ErrorHdl = ifx_pre_post_process_parse(pre_proc_hpf_prms, &pre_proc_hpf_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif
    ErrorHdl = ifx_pre_post_process_parse(sod_prms, &sod_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    mem_info_t am_mem_info;
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = ifx_am_predict_calculate_persistent_mem_size(NULL, NULL, NULL, &am_predict_sz);
#else
    ErrorHdl = ifx_am_predict_calculate_persistent_mem_size(itsi_am_prms_file_path, itsi_am_wts_file_path, its_am_bias_file_path, &am_predict_sz);
#endif
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    persistent_mem_size = am_predict_sz;
    am_mem_info.persistent_mem = am_predict_sz;
    am_mem_info.persistent_mem_pt = NULL;
#elif !defined(_MSC_VER) && defined(USE_MTB_ML)
    //AM NN model target for U55
    am_predict_sz = sizeof(mtb_ml_model_t); /* This persistent memory is allocated inside mtb_ml_model_init() function */
    /* MTB-ML global memory should be counted as persistent memory */
    am_predict_sz += ITSI_AM_LSTM_ARENA_SIZE + sizeof(mtb_ml_model_buffer_t) + N_SEQ * FEATURE_BUF_SZ + N_SEQ * FEATURE_BUF_SZ * sizeof(float);
    persistent_mem_size = am_predict_sz;
#endif
    ErrorHdl = ifx_pre_post_process_parse(denoise_prms, &feature_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_pre_post_process_parse(dfww_prms, &dfww_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;

    /* Step 2: Allocate memory */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    pre_proc_hpf_info.memory.persistent_mem_pt = (char*)malloc(pre_proc_hpf_info.memory.persistent_mem);
#else
    pre_proc_hpf_info.memory.persistent_mem_pt = NULL;
    pre_proc_hpf_info.memory.persistent_mem = 0;
    pre_proc_hpf_info.memory.scratch_mem = 0;
#endif
    sod_info.memory.persistent_mem_pt = (char*)malloc(sod_info.memory.persistent_mem);
    feature_info.memory.persistent_mem_pt = (char*)malloc(feature_info.memory.persistent_mem);
    dfww_info.memory.persistent_mem_pt = (char*)malloc(dfww_info.memory.persistent_mem);
    if (dfww_info.memory.persistent_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_POST_PROCESS_IP_COMPONENT_DFWWD, IFX_SP_ENH_ERR_PERSISTEMT_MEM);
    }
    persistent_mem_size += feature_info.memory.persistent_mem + dfww_info.memory.persistent_mem +
                              pre_proc_hpf_info.memory.persistent_mem + sod_info.memory.persistent_mem;

#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    am_mem_info.persistent_mem_pt = (char*)malloc(am_mem_info.persistent_mem);
    if (am_mem_info.persistent_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_AMPREDICT, IFX_SP_ENH_ERR_PERSISTEMT_MEM);
    }
    scrtach_sz = ifx_am_predict_calculate_scratch_mem_size();
    am_mem_info.scratch_mem = scrtach_sz;
#else
    scrtach_sz = 0;
#endif
    /* Get ITSI maximum scratch memory size */
    scrtach_sz = MAX(pre_proc_hpf_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(sod_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(feature_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(dfww_info.memory.scratch_mem, scrtach_sz);
    /* Allocate total scratch memory & shared with all components */
    feature_info.memory.scratch_mem_pt = (char*)malloc(scrtach_sz);
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    am_mem_info.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
#endif
    itsi_scratch.scratch_pad = feature_info.memory.scratch_mem_pt;
    itsi_scratch.scratch_size = scrtach_sz;
    itsi_scratch.scratch_cnt = 0;
    dfww_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    pre_proc_hpf_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    sod_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    if (feature_info.memory.scratch_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_DENOISE, IFX_SP_ENH_ERR_SCRATCH_MEM_NULL);
    }

    /* Step 3: Initialize itsi containers/objects */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    ErrorHdl = ifx_pre_post_process_init(pre_proc_hpf_prms, &pre_proc_hpf_obj, &pre_proc_hpf_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif
    ErrorHdl = ifx_pre_post_process_init(sod_prms, &sod_obj, &sod_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_pre_post_process_init(denoise_prms, &feature_obj, &feature_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = itsi_dfwwd_init(ww_model_ptr, &dfww_obj, &(dfww_info.memory), dfww_prms[DFWW_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#else
    ErrorHdl = read_itsi_ww_models(itsi_ww_model_file_path);
    if (ErrorHdl)
    {
        printf("ITSI read model failed, returning");
        return ErrorHdl;
    }
    ErrorHdl = itsi_dfwwd_init(WWmodeldata, &dfww_obj, &(dfww_info.memory), dfww_prms[DFWW_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif

#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    //AM NN model target for M55
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = ifx_am_predict_init(&am_obj, &am_mem_info, FEATURE_BUF_SZ, N_SEQ, FRAMES_HOP, N_PHONEMES,
        NULL, NULL, NULL);
#else
    ErrorHdl = ifx_am_predict_init(&am_obj, &am_mem_info, FEATURE_BUF_SZ, N_SEQ, FRAMES_HOP, N_PHONEMES,
        itsi_am_prms_file_path, itsi_am_wts_file_path, its_am_bias_file_path);
#endif
    if (ErrorHdl) return ErrorHdl;
#elif !defined(_MSC_VER) && defined(USE_MTB_ML)
    //AM NN model target for U55
    ErrorHdl = mtb_ml_inference_init();
#if defined(ENABLE_TRACE)
    printf("MTB-ML U55 init done with status = %x.\r\n", ErrorHdl);
#endif
    if (ErrorHdl) return ErrorHdl;
#endif
#if defined(ENABLE_TRACE)
    printf("ITSI persistent memory size: HPF=%d, SOD=%d, FEATURE=%d, AM Predict=%d, WW=%d, Total=%d; Scratch total=%d\r\n", pre_proc_hpf_info.memory.persistent_mem,
        sod_info.memory.persistent_mem, feature_info.memory.persistent_mem, am_predict_sz, dfww_info.memory.persistent_mem, persistent_mem_size, scrtach_sz);
#endif

    ErrorHdl = ifx_reset_itsi_wwd();
    return ErrorHdl;
}

uint32_t ifx_reset_itsi_wwd(void)
{
    int ErrorHdl = 0;

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    ErrorHdl = ifx_afe_hpf_reset(pre_proc_hpf_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif
    ErrorHdl = speech_utils_sod_reset(sod_prms, sod_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_itsi_feature_reset(feature_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_dfww_state_reset(dfww_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;

    ifx_reset_feature_buf();
    // Clear output_scores buffer
    memset(output_scores, 0, sizeof(float) * ((N_PHONEMES + 1) * AM_LOOKBACK));

    // counters and flags
    sodtrigcnt = -100;
    WWDresult = -2; // initial non-WWD state
    feature_buf_cnt = 0;

#ifdef ITSI_DEBUG
    frame_cnt = 0;
    SODcnt = 0;
#endif

    return IFX_SP_ENH_SUCCESS;
}

uint32_t ifx_free_itsi_wwd()
{
    int ErrorHdl = 0;

    // free ITSI persistent memory
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    free(pre_proc_hpf_obj);
    pre_proc_hpf_obj = NULL;
#endif
    free(sod_obj);
    sod_obj = NULL;
    ErrorHdl = itsi_feature_deinit(feature_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    free(feature_obj);
    feature_obj = NULL;
    free(dfww_obj);
    dfww_obj = NULL;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    ErrorHdl = ifx_am_predict_deinit(am_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    free(am_obj);
    am_obj = NULL;
#elif !defined(_MSC_VER) && defined(USE_MTB_ML)
    free(itsi_model_obj);
    itsi_model_obj = NULL;
#endif
    /* free scratch memory */
    ifx_mem_reset(&itsi_scratch);
    free(itsi_scratch.scratch_pad);
    itsi_scratch.scratch_pad = NULL;
    return IFX_SP_ENH_SUCCESS;
}

uint32_t ifx_run_itsi_wwd(short* mic_frame, int* ww_decison)
{
    uint32_t ret;

    // run SOD
    bool is_sod_detected;
    ret = ifx_sod_process((int16_t*)mic_frame, sod_obj, &is_sod_detected);
    if (ret != IFX_SP_ENH_SUCCESS) return ret;

#ifdef ITSI_DEBUG
    frame_cnt++;
#endif

    if (is_sod_detected)
    {
        if (WWDresult == -2)
        {// reset these everytime SOD is triggered and when WWDresult is -2.
            sodtrigcnt = 0;
        }
        WWDresult = 0;
        ret = ifx_reset_dfww(dfww_obj);
        if (ret != IFX_SP_ENH_SUCCESS) return ret;
#ifdef ITSI_DEBUG
        SODcnt++;
#if defined(ENABLE_TRACE)
        printf("ITSI SOD Detected: %d\n", SODcnt);
#endif
#endif
    }

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    /* run pre_process_hpf */
    ifx_time_pre_process(mic_frame, NULL, pre_proc_hpf_obj, IFX_PRE_PROCESS_IP_COMPONENT_HPF, mic_frame, NULL);
#endif

    /* do features and AM every frame for now */
    for (int i = 0; i < FRAME_SIZE_16K; i++)
    {
        xIn[i] = (float)mic_frame[i];
    }

    //2. Compute Features
    ret = itsi_feature_process_frame(xIn, feature_obj, features);
    if (ret != IFX_SP_ENH_SUCCESS)
    {
        return ret;
    }

#ifdef SAVE_FEATURES
    fwrite(features, sizeof(float), FEATURE_BUF_SZ, features_bin_file);
    num_features_hops++;
#endif

    //3. inference
#if !defined(_MSC_VER) && defined(USE_MTB_ML)
    float* buf_pt_base = mtb_ml_input_buffer;

    // update input buffer
    int hop = 1;
    int total_values = N_SEQ * FEATURE_BUF_SZ;
    int value_to_move = (N_SEQ - hop) * FEATURE_BUF_SZ;
    int offset = hop * FEATURE_BUF_SZ;
    for (int i = offset; i < total_values; i++)
    {
        buf_pt_base[i - offset] = buf_pt_base[i];
    }
    for (int i = 0; i < offset; i++)
    {
        buf_pt_base[i + value_to_move] = features[i];
    }

    feature_buf_cnt++;
    if (feature_buf_cnt == FRAMES_HOP)
    {
        /* shift the output_scores */
        for (int i = (N_PHONEMES + 1) * AM_LOOKBACK - 1; i >= (N_PHONEMES + 1); i--)
        {
            output_scores[i] = output_scores[i - (N_PHONEMES + 1)];
        }

        mtb_ml_utils_model_quantize(itsi_model_obj, buf_pt_base, data_feed_int);
        mtb_ml_model_rnn_reset_all_parameters(itsi_model_obj);
        /* Feed the Model */
        mtb_ml_model_run(itsi_model_obj, data_feed_int);

        // convert result_buffer to output_scores
        mtb_ml_utils_model_dequantize(itsi_model_obj, output_scores);
        feature_buf_cnt = 0;
    }
#else
    ret = ifx_am_predict_run(am_obj, &itsi_scratch, features, &feature_buf_cnt, output_scores, am_buffer_copy, false);
    if (ret != IFX_SP_ENH_SUCCESS) return ret;
#endif

    // post-inference processing
    * ww_decison = WWDresult;
    if ((WWDresult == 0) && (sodtrigcnt == 0))
    {// this has to be called every FRAMES_HOP !!
        ret = ifx_wwd(dfww_obj, &output_scores[(N_PHONEMES +1) * (AM_LOOKBACK - 1)], &WWDresult);
        if (ret != IFX_SP_ENH_SUCCESS) return ret;
        *ww_decison = WWDresult;
        if (WWDresult == 1)
        {
            WWDresult = -2;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
        else if (WWDresult == -1)
        {
            WWDresult = -2;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
    }

    sodtrigcnt++;
    if (sodtrigcnt == FRAMES_HOP)
    {/* this trigger DFWW to be called every FRAMES_HOP !! */
        sodtrigcnt = 0;
    }

    return 0;
}
