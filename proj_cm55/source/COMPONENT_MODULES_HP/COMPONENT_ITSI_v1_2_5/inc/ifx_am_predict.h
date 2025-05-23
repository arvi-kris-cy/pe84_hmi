/******************************************************************************
* File Name: ifx_am_predict.h
*
* Description: This file contains Infineon AM predict header
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
* Include guard
*******************************************************************************/
#ifndef __IFX_AM_PREDICT_H
#define __IFX_AM_PREDICT_H

uint32_t ifx_am_predict_init(void** container, mem_info_t* mem_infoPt, int32_t n_features, int32_t n_seq, int32_t hop, int32_t am_out_sz,
    char* mdl_prms_file_path, char* mdl_wts_file_path, char* mdl_bias_file_path);
uint32_t ifx_am_predict_calculate_persistent_mem_size(char* mdl_prms_file_path, char* mdl_wts_file_path, char* mdl_bias_file_path, int32_t* persist_mem_size);
uint32_t ifx_am_predict_calculate_scratch_mem_size(void);
uint32_t ifx_am_predict_reset(void* containerPt);
uint32_t ifx_am_predict_deinit(void* containerPt);
uint32_t ifx_am_predict_run(void* containerPt, ifx_scratch_mem_t* scratch_ptr, float* features, int* buf_cnt, float* output_scores, float* am_buffer_copy, bool eof);
#endif /* __IFX_AM_PREDICT_H */
