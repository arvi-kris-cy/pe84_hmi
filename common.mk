################################################################################
# \file common.mk
# \version 1.0
#
# \brief
# Settings shared across all projects.
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

MTB_TYPE=PROJECT

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make library-manager' from command line), which will also update Eclipse IDE launch
# configurations.
TARGET=KIT_PSE84_HMI

# Name of toolchain to use. Options include:
# ARM     - Arm Compiler (must be installed separately). This CE supports only Arm Compiler.

TOOLCHAIN=ARM

ifeq ($(TOOLCHAIN),ARM)
#   Uncomment after importing the project or set this variable as env variable in the PC.
#    For Eg) CY_COMPILER_ARM_DIR=C:/Program Files/ArmCompilerforEmbedded6.22
endif

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
# 
# If CONFIG is manually edited, ensure to update or regenerate launch configurations 
# for your IDE.
CONFIG=Debug

###########################################################################################
##################### Audio Pipeline Configuration ########################################

# Configuration of the Audio Pipeline. Options are:
#
# BATTERY_POWERED_LOCAL_VOICE -- Includes Staged Voice Control that uses both
# Low Power Core and High Performance Core for Wake Word Detection.
#
# MAINS_POWERED_LOCAL_VOICE -- Uses only High Performance Core.

CONFIG_APP_PIPELINE=MAINS_POWERED_LOCAL_VOICE


#########################################################################################
##################### Voice Core ########################################################

# Voice Core functionality. Options are
# LIMITED - Limited time functionality. (Default)
# FULL    - Full functionality.

CONFIG_VOICE_CORE_MODE=LIMITED

##########################################################################################
##################### Inferencing Library ################################################

# Option to choose the inferencing library on High Performance core.
# ITSI_M55_U55  - Infineon Inferencing engine (ITSI version 1.2.5) on M55 and U55.
# SENSORY       - Sensory Inferencing engine on M55 and U55. (Only with Mains powered configuration)
# DISABLED      - Disable inferencing on M55 and U55.

CONFIG_INFERENCING_LIB=ITSI_M55_U55

#########################################################################################


##########################################################################################
##################### USB Audio Debugging & Tuning  ######################################

# Option to enable USB Audio for debugging and tuning the audio pipeline.
#
# PDM_MIC_IN        - Enable USB debugging and tuning with PDM mic input.
# USB_AUD_IN        - Enable USB debugging and tuning with USB audio input.
# DISABLED          - Disable USB debugging and tuning. (Default)    

DEBUG_TUNING_MODE=DISABLED

#########################################################################################


MSP_STACK_SIZE=0x00004000

LDFLAGS+=--predefine="-DAPP_MSP_STACK_SIZE=$(MSP_STACK_SIZE)"

# Search path for
# 1. Common source code shared between CM33 and CM55 cores.
# 2. Third party licensed code.

SEARCH+=../common_modules
SEARCH+=../ce_internal


##########################################################################################

include ../common_app.mk
