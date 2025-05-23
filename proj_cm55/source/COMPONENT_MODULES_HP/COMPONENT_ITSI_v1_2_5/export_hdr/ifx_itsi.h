/******************************************************************************
* File Name: ifx_itsi.h
*
* Description: This file contains public high level interface for Infineon ITSI
*
* Related Document: See README.md
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
#ifndef __IFX_ITSI_H
#define __IFX_ITSI_H

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
* Include header file
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#ifdef EMBEDDED_NO_FILE
extern const char WWmodeldata[];
extern const char CMDmodeldata[];
extern const char NMBmodeldata[];
#else
extern char WWmodeldata[];
extern char CMDmodeldata[];
extern char NMBmodeldata[];
#endif
extern void* dfww_obj;
extern void* dfcmd_obj;
extern volatile bool ptt_flag; /* Application team is responsible to implementation push buttom and set this global variable */

#define IFX_SP_ITSI_VERSION_MAJOR            1
#define IFX_SP_ITSI_VERSION_MINOR            2
#define IFX_SP_ITSI_VERSION_PATCH            5
#define IFX_SP_ITSI_VERSION                  125

/******************************************************************************
* Function prototype
******************************************************************************/
/**
 * \addtogroup ITSI API
 * @{
 */

 /**
  * \brief : ifx_init_itsi() is the API function to initialize itsi.
  * \brief : ifx_init_itsi_wwd() is the API function to initialize itsi for wwd only.
  *
  * \param[in]   ww_model_ptr     : Pointer to dfwwd model that contains dfww model parameters
  * \param[in]   cmd_model_ptr    : Pointer to dfcmd model that contains dfcmd model parameters
  * \param[in]   nmb_model_ptr    : Pointer to dfnmb model that contains dfnmb model parameters
  * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
  *                                 Please note error code is 8bit LSB, line number where the error happened in
  *                                 code is in 16bit MSB, and its IP component index if applicable will be at
  *                                 bit 8 to 15 in the combined 32bit return value.
 */
#ifdef EMBEDDED_NO_FILE 
uint32_t ifx_init_itsi(const char* ww_model_ptr, const char* cmd_model_ptr, const char* nmb_model_ptr);
uint32_t ifx_init_itsi_wwd(const char* ww_model_ptr);
#else
uint32_t ifx_init_itsi(char* itsi_am_prms_file_path, char* itsi_am_wts_file_path, char* its_am_bias_file_path,
    char* itsi_ww_model_file_path, char* itsi_cmd_model_file_path, char* itsi_nmb_model_file_path);
uint32_t ifx_init_itsi_wwd(char* itsi_am_prms_file_path, char* itsi_am_wts_file_path, char* its_am_bias_file_path, char* itsi_ww_model_file_path);
#endif

/**
 * \brief : ifx_reset_itsi() is the API function to reset itsi.
 *
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_reset_itsi();

/**
 * \brief : ifx_reset_itsi_wwd() is the API function to reset itsi wwd only.
 *
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_reset_itsi_wwd();

/**
 * \brief : ifx_reset_feature_buf() is the API function to reset itsi feature buffer.
 *
 * \return                       : No return 
*/
void ifx_reset_feature_buf(void);

/**
 * \brief : ifx_run_itsi() is the API function to run itsi process.
 *
 * \param[out] ww_decison        : pointer to Wake Word (WW) detection is written, 1=WW detected, -1=No WW detected, 0=indecision
 * \param[out] cmd_decison       : pointer to command detection is written, 1=CMD detected, -1=No CMD detected, 0=indecision
 * \param[in]  eof               : Pointer to eod of file (input) indicator
 * \param[in]  in                : Pointer to 10ms frame PCM samples input
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_run_itsi(short* in, bool eof, int* ww_decison, int* cmd_decision);

/**
 * \brief : ifx_run_itsi_wwd() is the API function to run itsi wwd only process.
 *
 * \param[out] ww_decison        : pointer to Wake Word (WW) detection is written, 1=WW detected, -1=No WW detected, 0=indecision
 * \param[in]  in                : Pointer to 10ms frame PCM samples input
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_run_itsi_wwd(short* in, int* ww_decison);

/**
 * \brief : ifx_free_itsi() is the API function to release itsi memory.
 *
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_free_itsi();

/**
 * \brief : ifx_free_itsi_wwd() is the API function to release itsi wwd only memory.
 *
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_free_itsi_wwd();

/**
 * \brief : ifx_get_command() is the API function to get detected command text process.
 *
 * \param[out] commandtext       : pointer to character string of detected command text
 * \param[in]  cmdStrucPt        : Pointer to command data structure container
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_get_command(void* DFcmdStrucPt, char* commandtext);

/**
 * \brief : ifx_get_command_indices_and_numbers() is the API function to get detected command details.
 * This function returns the command index or indices (*indices) that were detected.
 * These indices map back into the original command list that was trained for
 * detection. Note that one or two indices may be returned, depending on the
 * structure of the original command.  The number of indices actually returned is
 * given by the value of the returned parameter (*Nindices).
 *
 * This function also returns any numbers and appropriate units of such numbers. As
 * there are many types of number fields supported with different formats, the numbers
 * (such as time) that the higher level calling function can reformat or convert to
 * integers as desired.  The total number of numbers returned in the (*numbers) field is
 * given by the return value (*Nnumbers).
 *
 * The units corresponding to each number are returned in the char array
 * ((*units)[20]).  Based on the command recognized, the higher layers know what any
 * subsequent number returned refers to and whether or not that number includes any
 * units. The total number of units returned (which may not equal the number of numbers)
 * is returned in (* Nunits).
 *
 * As of the current release, the numbers fields supported include:
 * - Numbers - the format is 'xxx', NNumbers = 1
 *       units: degree(s), percent, level(s), hour(s), minute(s), second(s),
 *              day(s), no units
 *              Nunits = 1
 *
 * - Time - the format is 'xx:yy' where xx is the 12-hour hours, and yy is minutes, NNumbers = 1
 *       units: 'AM' or 'PM'
 *              Nunits = 1
 *
 * - Duration - the format is 'xxx', NNumbers = 1 to 3
 *       units: hour(s), minute(s), second(s)
 *              Nunits = NNumbers = 1 to 3
 *
 * - Ordinals - the format is 'xxx', NNumbers = 1
 *       units: none
 *              Nunits = 0
 *
 * \param[out] indices           : pointer to detected command indices
 * \param[out] Nindices          : pointer to an integer specifies the number of detected command indices (1 or 2) is returned.
 * \param[out] numbers           : pointer to array of char of size 3 of detected numbers in char, each array of length 20 chars.
 * \param[out] Nnumbers          : pointer to integer specifies the number of numbers (-1 to 3) that are returned, with -1 represents
                                   no numbers was recognized but expected.
 * \param[out] units             : pointer to array of char of size 3 of detected unit, each array of length 20 chars.
 * \param[out] Nunits            : pointer to integer specifies the number of detected units (-1 to 3) that are returned, with -1 represents
                                   no number unit was recognized but expected.
 * \param[in]  cmdStrucPt        : Pointer to command data structure container
 * \return                       : Return 0 when success, otherwise return error code from specific ifx itsi proces module.
 *                                 Please note error code is 8bit LSB, line number where the error happened in
 *                                 code is in 16bit MSB, and its IP component index if applicable will be at
 *                                 bit 8 to 15 in the combined 32bit return value.
*/
uint32_t ifx_get_command_indices_and_numbers(void* DFcmdStrucPt, int* indices, int* Nindices, char (*numbers)[20], int* Nnumbers, char (*units)[20], int* Nunits);


#ifdef __cplusplus
}
#endif

#endif /* __IFX_ITSI_H */
