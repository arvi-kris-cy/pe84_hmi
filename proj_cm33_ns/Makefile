################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configuring the Cortex-M33 core project
#
################################################################################
# \copyright
# Copyright 2025, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk

################################################################################
# Basic Configuration
###############################################################################

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=proj_cm33_ns
APPTYPE=flash

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS += FREERTOS RTOS_AWARE  

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

CORE=CM33
CORE_NAME=CM33_0

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES+=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES+=

# Add additional defines to the build process (without a leading -D).
DEFINES += CY_RETARGET_IO_CONVERT_LF_TO_CRLF

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS+=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS+=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS+=

# Additional / custom linker flags.
LDFLAGS+=

# Additional / custom libraries to link in to the application.
LDLIBS+=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=
# Custom pre-build commands to run.
PREBUILD+=

# Custom post-build commands to run.
POSTBUILD+=

# IAR/ARM compiler uses the CBUS address for compilation. Writing to CBUS address is not allowed
# Relocate to SAHB address for programming the image. (This step is only required for CM33 core) 
ifneq (,$(filter $(TOOLCHAIN),ARM IAR))
APP_START=0x60340400
POSTBUILD=$(CY_TOOL_edgeprotecttools_EXE_ABS) bin2hex --image $(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).bin --output $(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).hex --offset $(APP_START)
endif

################################################################################
# Application specific Configurations to use COMPONENTS, DEFINES and flags.
################################################################################

#Application Logger
COMPONENTS += APP_LOGGER 

#Low Power Core (CM33) is used only with CONFIG_APP_PIPELINE=BATTERY_POWERED_LOCAL_VOICE
ifeq ($(CONFIG_APP_PIPELINE),BATTERY_POWERED_LOCAL_VOICE)
    DEFINES += BATTERY_POWERED_LOCAL_VOICE

    ifneq ($(CONFIG_INFERENCING_LIB),DISABLED)
        DEFINES += HP_INFERENCING_ENABLE
    endif
    
    ifeq ($(CONFIG_INFERENCING_LIB),ITSI_M55_U55)
        DEFINES += ITSI_INFERENCING
    endif
    
    ifeq ($(CONFIG_INFERENCING_LIB),SENSORY)
        DEFINES += SENSORY_INFERENCING
    endif
#Modules for CM33 application    
    COMPONENTS += MODULES_LP
#USB component.
    COMPONENTS += USBD_BASE 
#CMSIS DSP for voice processing.
    COMPONENTS += CMSIS_DSP
#Audio input
    COMPONENTS += AUDIO_INPUT
#Profiler
#    COMPONENTS += PROFILER
#Push to talk control via User Button.
    COMPONENTS += PUSH_TO_TALK
#Conversion utilities
    COMPONENTS += AUDIO_FORMAT_UTILS 
#FreeRTOS related defines.
    DEFINES += CY_RTOS_AWARE #CY_USING_HAL
#Floating point selection.  
    VFP_SELECT = hardfp
#Audio Front End Configuration
    COMPONENTS += AFE_CONFIGURATION
#Enable Staged Voice Control Middleware       
    DEFINES += ENABLE_SVC_LP_MW
#Enable High Pass Filter - software based.    
#    DEFINES += ENABLE_IFX_PRE_PROCESS_HPF #Not used in E2 release.

# Wake word model in staged voice control   
    COMPONENTS += SVC_OK_INFINEON

#Staged Voice Control Shared buffer configuration and IPC configuration    
    ENABLE_SVC_IPC_BY_PDL_PIPE=Y
#Inter-process communication between Low Power Core and High Performance Core.
    ifeq ($(ENABLE_SVC_IPC_BY_PDL_PIPE), Y)
#Inter-core pipe
        COMPONENTS += INTERCORE_PIPE
        DEFINES += INTERCORE_PIPE
        DEFINES += ENABLE_SVC_IPC_BY_PDL_PIPE
    endif

    DEFINES += SVC_SHARED_BUFFER_SECTION=1
     
#In-case Low Power wake-word is to be overriden.
#    DEFINES += LPWWD_OVERRIDE #Override Low Power WakeWord.

#Low power wake word detection configuration
    DEFINES += EMBEDDED_DEV ENABLE_IFX_LPWWD 
    COMPONENTS += IFX_LPWWD ML_INT8x8 
    DEFINES += TF_LITE_STATIC_MEMORY ML_INT8x8

    DEFINES += ENABLE_SVC_ML_MW_SUPPORT

#TensorFlowLite Micro Interpreter-Less mode.    
    ML_TFLM_MODE=ML_TFLM_LESS

    ifeq ($(ML_TFLM_MODE),ML_TFLM_LESS)
        COMPONENTS += ML_TFLM_LESS
        DEFINES += TF_LITE_STRIP_ERROR_STRINGS
        DEFINES += TF_LITE_MICRO_USE_OFFLINE_OP_USER_DATA
        NN_INFERENCE_ENGINE=tflm_less
        COMPONENTS += ML_T_LESS_NNL2
        INCLUDES+=$(SEARCH_cmsis)/COMPONENT_CMSIS_NN/Include
    else
        COMPONENTS += ML_TFLM
        NN_INFERENCE_ENGINE=tflm
    endif

    NN_MODEL_NAME=OK_INFINEON_INT8

    NN_TYPE=int8x8
    DEFINES +=MODEL_NAME=$(NN_MODEL_NAME)


#To enable Tflitemicro operations in CM33 only (CPU mode),
    # COMPONENTS += ML_CM33_ONLY   
#To enable Tflitemicro operation with NNLITE mode,
    COMPONENTS += NNLITE2
#For Speech Onset Detection
    DEFINES += ENABLE_IFX_SOD

#Application control macro definition
    ifeq ($(DEBUG_TUNING_MODE),DISABLED)
#Actual application flow for Battery Powered Application
        DEFINES += BATTERY_POWERED_LOCAL_VOICE_APP 
        COMPONENTS += BATTERY_POWERED_APP_LP
    else
#Debug and tuning flow    
        DEFINES += DEBUG_TUNE_BATTERY_APP
 
        COMPONENTS += TUNE_BATTERY_APP
        COMPONENTS += USB_AUDIO
        ifeq ($(DEBUG_TUNING_MODE),PDM_MIC_IN)
            DEFINES += ENABLE_MIC_INPUT_FEED
        endif
#USB input is stereo channel only.        
        ifeq ($(DEBUG_TUNING_MODE),USB_AUD_IN)
            DEFINES += USB_AUDIO_INPUT
        endif

    endif
    
endif

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field 
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by 
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level 
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# IDE provided compiler by default).
CY_COMPILER_PATH?=

CY_AVC_PATH=$(SEARCH_audio-voice-core)

ifeq ($(CONFIG_APP_PIPELINE),MAINS_POWERED_LOCAL_VOICE)
CY_IGNORE += $(SEARCH_ml-middleware)
CY_IGNORE += $(SEARCH_ml-tflite-micro)
CY_IGNORE += $(SEARCH_nnkernel)
CY_IGNORE += $(SEARCH_emusb-device)
CY_IGNORE += $(SEARCH_speech-onset-detection)
CY_IGNORE += $(SEARCH_staged-voice-control)
CY_IGNORE += imports/emusb-device
CY_IGNORE += ${CY_AVC_PATH}/lib/SP_ENH/demo
CY_IGNORE += ${CY_AVC_PATH}/lib/SP_ENH/full
endif

ifeq ($(CONFIG_VOICE_CORE_MODE),FULL)
    CY_IGNORE += ${CY_AVC_PATH}/lib/SP_ENH/demo
endif

ifeq ($(CONFIG_VOICE_CORE_MODE),LIMITED)
    CY_IGNORE += ${CY_AVC_PATH}/lib/SP_ENH/full
endif

include $(CY_TOOLS_DIR)/make/start.mk
