/******************************************************************************
* File Name : pdm_mic_interface.c
*
* Description :
* Code for PDM mic interface. Used by both CM33 and CM55 cores.
********************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_log.h"

#include "pdm_mic_interface.h"
#include "audio_input_configuration.h"
#include "app_logger.h"
#include <math.h>

/*******************************************************************************
* Macros
*******************************************************************************/
#ifdef ENABLE_STEREO_INPUT_FEED
#define MIC_MODE                         (2u)
#else
#define MIC_MODE                         (1u)
#endif /* ENABLE_STEREO_INPUT_FEED */


#define PDM_PCM_HW_FIFO_SIZE             (64u)
#define PDM_PCM_FRAME_INTR_CNT           (5u)
#define MIC_RX_SAMPLES_COUNT             (160*MIC_MODE)

#define PDM_FIR1_GAIN_CONST              (13921L)
#define PDM_FIR_MAX_SCALE_VALUE          (31)
#define PDM_GAIN_DB_STEPS_CONST          (0.5f)
#define PDM_GAIN_CONVERT_CONST_2         (2)
#define PDM_GAIN_CONVERT_CONST_10        (10)
#define PDM_GAIN_CONVERT_CONST_20        (20)
#define PDM_SET_GAIN_ERROR               (-1)


/* The number of interrupts to get frame of 10 msec samples.
    5 interrupts of 2msec makes 10msec frame */
/* 10msec data is 320 samples in STEREO mode*/
/* 10msec data is 160 samples in STEREO mode*/
#ifdef ENABLE_STEREO_INPUT_FEED
#define HALF_FIFO_SIZE              (PDM_PCM_HW_FIFO_SIZE)
#else
#define HALF_FIFO_SIZE              (PDM_PCM_HW_FIFO_SIZE/2)
#endif /* ENABLE_STEREO_INPUT_FEED */

/*******************************************************************************
* Global Variables
*******************************************************************************/
int16_t mic_audio_app_buffer_ping[MIC_RX_SAMPLES_COUNT] = {0};
int16_t mic_audio_app_buffer_pong[MIC_RX_SAMPLES_COUNT] = {0};

volatile bool pdm_pcm_flag = false;
volatile bool pdm_pcm_frame_flag = false;
volatile uint8_t pdm_pcm_intr_cnt = 0;
int16_t audio_frame[HALF_FIFO_SIZE] = {0};

int16_t* ping_pong_local_pointer = NULL;
int16_t* ping_pong_buffer_pointer = NULL;

volatile int16_t *audio_data_ptr = NULL;

/* PDM/PCM interrupt configuration parameters */
const cy_stc_sysint_t PDM_IRQ_cfg = {
    .intrSrc = (IRQn_Type)CYBSP_PDM_CHANNEL_3_IRQ,
    .intrPriority = PDM_PCM_INTR_PRIORITY
};

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
#ifdef BATTERY_POWERED_LOCAL_VOICE
    extern void audio_mic_data_feed_cm33(int16_t *audio_data);
#else
    extern void audio_mic_data_feed_cm55(int16_t *audio_data);
#endif /* BATTERY_POWERED_LOCAL_VOICE */

static inline int32_t convert_pdm_pcm_gain_to_scale(int16_t gain_val);

/*******************************************************************************
 * Function Name: pdm_interrupt_handler
 ********************************************************************************
 * Summary:
 *  PDM PCM converter ISR callback.
 *  Populates ping/pong audio data buffer every 10msec.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void pdm_interrupt_handler(void)
{
    static bool ping_pong = false;
    volatile uint32_t int_stat;

    /* Choose the ping pong buffer */
    if(pdm_pcm_intr_cnt == 0)
    {
        if (ping_pong)
        {
            ping_pong_local_pointer = mic_audio_app_buffer_ping;
        }
        else
        {
            ping_pong_local_pointer = mic_audio_app_buffer_pong;
        }
        ping_pong_buffer_pointer = ping_pong_local_pointer;
    }

    int_stat = Cy_PDM_PCM_Channel_GetInterruptStatusMasked(PDM0, RIGHT_CH_INDEX);
    if(CY_PDM_PCM_INTR_RX_TRIGGER & int_stat)
    {
        for(uint8_t i=0; i < RX_FIFO_TRIG_LEVEL; i++)
        {
#ifdef ENABLE_STEREO_INPUT_FEED
            int32_t data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, LEFT_CH_INDEX);
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;

            data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, RIGHT_CH_INDEX);
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;
#else
            int32_t data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, LEFT_CH_INDEX);
            data = (int32_t)Cy_PDM_PCM_Channel_ReadFifo(PDM0, RIGHT_CH_INDEX);
            *(ping_pong_buffer_pointer) = (int16_t)(data);
            ping_pong_buffer_pointer++;
#endif
        }

        if (pdm_pcm_intr_cnt < PDM_PCM_FRAME_INTR_CNT)
        {
            pdm_pcm_intr_cnt++;
        }

        if (PDM_PCM_FRAME_INTR_CNT == pdm_pcm_intr_cnt)
        {
            pdm_pcm_intr_cnt = 0;
#ifdef BATTERY_POWERED_LOCAL_VOICE
            audio_mic_data_feed_cm33(ping_pong_local_pointer);
#else
            audio_mic_data_feed_cm55(ping_pong_local_pointer);
#endif /* BATTERY_POWERED_LOCAL_VOICE */
            ping_pong = !ping_pong;
            if (ping_pong)
            {
                ping_pong_local_pointer = mic_audio_app_buffer_ping;
            }
            else
            {
                ping_pong_local_pointer = mic_audio_app_buffer_pong;
            }
            ping_pong_buffer_pointer = ping_pong_local_pointer;
        }

        Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_RX_TRIGGER);
    }

    if((CY_PDM_PCM_INTR_RX_FIR_OVERFLOW | CY_PDM_PCM_INTR_RX_OVERFLOW |
        CY_PDM_PCM_INTR_RX_IF_OVERFLOW | CY_PDM_PCM_INTR_RX_UNDERFLOW) & int_stat)
    {
        /*printf("Error: channel %u RX FIFO Error INTR_STATUS = %lu\n",
               (unsigned int) RIGHT_CH_INDEX, (unsigned long int) int_stat);*/
        Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_MASK);
    }
}

/*******************************************************************************
* Function Name: pdm_mic_interface_init
********************************************************************************
* Summary:
* Initalize the PDM block with the required settings for PSOC Edge.
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/
cy_rslt_t pdm_mic_interface_init(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize PDM/PCM block */
    cy_en_pdm_pcm_status_t volatile status = Cy_PDM_PCM_Init(PDM0, &CYBSP_PDM_config);

    /* Enable PDM channel, we will activate channel for record later */
    Cy_PDM_PCM_Channel_Enable(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_Channel_Enable(PDM0, RIGHT_CH_INDEX);

    /* Initialize PDM/PCM channel 0 -Left, 1 -Right */
    Cy_PDM_PCM_Channel_Init(PDM0, &LEFT_CH_CONFIG, (uint8_t)LEFT_CH_INDEX);
    Cy_PDM_PCM_Channel_Init(PDM0, &RIGHT_CH_CONFIG, (uint8_t)RIGHT_CH_INDEX);

    /* Set the gain for both left and right channels. */
    set_pdm_pcm_gain(PDM_MIC_GAIN_VALUE);

    /* As registred for right channel, clear and set maks for it. */
    Cy_PDM_PCM_Channel_ClearInterrupt(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_MASK);
    Cy_PDM_PCM_Channel_SetInterruptMask(PDM0, RIGHT_CH_INDEX, CY_PDM_PCM_INTR_MASK);

    /* Register the PDM/PCM hardware block IRQ handler */
    if(CY_SYSINT_SUCCESS != Cy_SysInt_Init(&PDM_IRQ_cfg, &pdm_interrupt_handler))
    {
        printf("PDM/PCM Initialization has failed! \r\n");
        CY_ASSERT(0);
    }

    printf("PDM/PCM Initialization Successful \r\n");
    NVIC_ClearPendingIRQ(PDM_IRQ_cfg.intrSrc);
    NVIC_EnableIRQ(PDM_IRQ_cfg.intrSrc);

    app_pdm_pcm_activate();

    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: pdm_mic_interface_deinit
********************************************************************************
* Summary:
*  De-initializes PDM-PCM block.
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/
cy_rslt_t pdm_mic_interface_deinit(void)
{
    return CY_RSLT_SUCCESS;
}


/*******************************************************************************
 * Function Name: app_pdm_pcm_activate
 ********************************************************************************
* Summary: This function activates the left and righ channel.
*
* Parameters:
*  None
*
* Return:
*  none
*
*******************************************************************************/
void app_pdm_pcm_activate(void)
{
    /* Activate recording from channel after init Activate Channel */
    Cy_PDM_PCM_Activate_Channel(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_Activate_Channel(PDM0, RIGHT_CH_INDEX);
}

/*******************************************************************************
* Function Name: app_pdm_pcm_deactivate
********************************************************************************
* Summary: This function activates the left and righ channel.
*
* Parameters:
*  none
*
* Return :
*  none
*
*******************************************************************************/
void app_pdm_pcm_deactivate(void)
{
    Cy_PDM_PCM_DeActivate_Channel(PDM0, LEFT_CH_INDEX);
    Cy_PDM_PCM_DeActivate_Channel(PDM0, RIGHT_CH_INDEX);
}

/*******************************************************************************
 * Function Name: convert_pdm_pcm_gain_to_scale
 ********************************************************************************
 * Summary:
 *  Function that converts the input gain value to FIR scale value.
 *
 * Parameters:
 *  gain_val    : Input gain value in 0.5 dB steps. Should be in the range
 *                PDM_PCM_MIN_GAIN <= gain_val <= PDM_PCM_MAX_GAIN
 *
 * Return:
 *  int32_t: FIR scale value
 *
 *******************************************************************************/
static inline int32_t convert_pdm_pcm_gain_to_scale(int16_t gain_val)
{
    /* The formula for gain in db is db = 20 * log_10(FIR1_GAIN / 2^scale),
    *  where FIR1_GAIN = 13921.
     * Solving for scale, we get: scale = log_2(FIR1_GAIN / 10^(db / 20))
     */

    /* Cmath only provides ln and log10, need to compute log_2 in terms of those */
    /* Gain is specified in 0.5 db steps in the interface */
    float desired_gain = ((float)gain_val) * PDM_GAIN_DB_STEPS_CONST;
    float inner_value = PDM_FIR1_GAIN_CONST / (powf(PDM_GAIN_CONVERT_CONST_10,
                        (desired_gain / PDM_GAIN_CONVERT_CONST_20)));
    float scale = logf(inner_value) / logf(PDM_GAIN_CONVERT_CONST_2);
    int32_t scale_rounded = (uint8_t)(scale + PDM_GAIN_DB_STEPS_CONST);
    return scale_rounded;
}

/*******************************************************************************
 * Function Name: set_pdm_pcm_gain
 ********************************************************************************
 * Summary:
 *  Function to set the gain for the PDM PCM block for stereo channels in steps
 *  of 0.5 dB.
 *
 * Parameters:
 *  gain  : Gain value in 0.5 dB steps. Should be in the range
 *          PDM_PCM_MIN_GAIN <= gain <= PDM_PCM_MAX_GAIN
 *
 * Return:
 *  cy_rslt_t: Result of the function
 *
 *******************************************************************************/
cy_rslt_t set_pdm_pcm_gain(int16_t gain)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_en_pdm_pcm_ch_fir1_decimcode_t decim_code;
    int32_t fir1_scale_value;

    if (gain < PDM_PCM_MIN_GAIN || gain > PDM_PCM_MAX_GAIN)
    {
        result = PDM_SET_GAIN_ERROR;
    }

    if(CY_RSLT_SUCCESS == result)
    {
        fir1_scale_value = convert_pdm_pcm_gain_to_scale(gain);
        if((fir1_scale_value < 0) || (fir1_scale_value > PDM_FIR_MAX_SCALE_VALUE))
        {
            result = PDM_SET_GAIN_ERROR;
        }
    }

    if(CY_RSLT_SUCCESS == result)
    {
        decim_code = (cy_en_pdm_pcm_ch_fir1_decimcode_t) CY_PDM_PCM_CHAN_FIR1_DECIM_3;

        Cy_PDM_PCM_Channel_Set_Fir1(PDM0, LEFT_CH_INDEX, decim_code,
                                    fir1_scale_value);
        Cy_PDM_PCM_Channel_Set_Fir1(PDM0, RIGHT_CH_INDEX, decim_code,
                                    fir1_scale_value);
    }

    return result;
}

/* [] END OF FILE */
