################################################################################
# \file bsp.mk
#
# \brief
# Define the BSP for the KIT_PSE84_EVAL_EPC2
#
################################################################################
# \copyright
# Copyright 2023-2025 Cypress Semiconductor Corporation (an Infineon company) or
# an affiliate of Cypress Semiconductor Corporation
#
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

ifeq ($(WHICHFILE),true)
$(info Processing $(lastword $(MAKEFILE_LIST)))
endif

# Any additional components to apply when using this board.
BSP_COMPONENTS:=HCI-UART WIFI_INTERFACE_SDIO SM CYW55513_MOD_PSE84_SOM wlbga_iPA_sLNA_ANT0_LHL_XTAL_IN

# Any additional defines to apply when using this board.
BSP_DEFINES+=CY_WIFI_COUNTRY='WHD_COUNTRY_UNITED_STATES'
BSP_DEFINES+=CYBSP_WIFI_WL_HOSTWAKE_DRIVE_MODE=MTB_HAL_GPIO_DRIVE_OPENDRAINDRIVESLOW
BSP_DEFINES+=CYBSP_WIFI_WL_HOSTWAKE_INIT_STATE=WHD_TRUE

# Path to the flash loaders to patch for this board
CY_QSPI_FLM_DIR?=$(MTB_TOOLS__TARGET_DIR)/config/FlashLoaders/

# Path to the patched flash loaders for this board
CY_QSPI_FLM_DIR_OUTPUT?=$(MTB_TOOLS__TARGET_DIR)/config/GeneratedSource/

ENABLE_SEC_API_LIB?=0

APPTYPE?=flash
VCORE_ATTRS?=NON_SECURE
CORE?=CM33
ifeq ($(ENABLE_SEC_API_LIB), 1) 
SEC_API_LINK_FILE_NAME=sec_api_link.o
endif 

ifneq (,$(filter $(TOOLCHAIN),GCC_ARM LLVM_ARM))
# To pick up pse84_common.ld
LDFLAGS += -Xlinker -L $(MTB_TOOLS__TARGET_DIR)/COMPONENT_CM33/TOOLCHAIN_$(TOOLCHAIN)
LDFLAGS += -Xlinker -L $(MTB_TOOLS__TARGET_DIR)/config/GeneratedSource
else ifeq ($(TOOLCHAIN),ARM)
LDFLAGS += --undefined=__scatterload_copy
endif

# Specify the path to the linker script to use
ifneq (,$(filter $(TOOLCHAIN),GCC_ARM LLVM_ARM))
    BSP_LINKER_SCRIPT_EXT:=ld
else ifeq ($(TOOLCHAIN),ARM)
    BSP_LINKER_SCRIPT_EXT:=sct
else ifeq ($(TOOLCHAIN),IAR)
    BSP_LINKER_SCRIPT_EXT:=icf
endif


ifeq ($(VCORE_ATTRS),NON_SECURE)

ifeq ($(CORE), CM33)
MTB_BSP__LINKER_SCRIPT:=$(MTB_TOOLS__TARGET_DIR)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/pse84_ns_cm33.$(BSP_LINKER_SCRIPT_EXT)
else
MTB_BSP__LINKER_SCRIPT:=$(MTB_TOOLS__TARGET_DIR)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/pse84_ns_cm55.$(BSP_LINKER_SCRIPT_EXT)
endif

COMPONENTS+=NON_SECURE_DEVICE

ifeq ($(CORE), CM33)
ifeq ($(ENABLE_SEC_API_LIB), 1)
LDLIBS+=$(MTB_TOOLS__TARGET_DIR)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/COMPONENT_PREBUILT_SECURE_APP/$(SEC_API_LINK_FILE_NAME)
endif 
endif

else
MTB_BSP__LINKER_SCRIPT:=$(MTB_TOOLS__TARGET_DIR)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/pse84_s_cm33.$(BSP_LINKER_SCRIPT_EXT)
COMPONENTS+=SECURE_DRIVERS SECURE_DEVICE
endif

#Generate binary image
ifeq ($(TOOLCHAIN),ARM)
CY_BSP_POSTBUILD+=$(MTB_TOOLCHAIN_ARM__BASE_DIR)/bin/fromelf --output=$(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).bin --bincombined $(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).$(MTB_RECIPE__SUFFIX_TARGET);
else
CY_BSP_POSTBUILD+=$(MTB_TOOLCHAIN_GCC_ARM__BASE_DIR)/bin/arm-none-eabi-objcopy -O binary $(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).$(MTB_RECIPE__SUFFIX_TARGET) $(MTB_TOOLS__OUTPUT_CONFIG_DIR)/$(APPNAME).bin;
endif

#Work around for H1 Hosted mode
DISABLE_COMPONENTS+=CAT5

#Work around for BSP assistant limitation in make recipe calculation
SEARCH_recipe-make-cat1d=$(SEARCH_mtb-device-support-pse8xxgp)/make
BSP_DEVICE_SUPPORT_LIB:=mtb-device-support-pse8xxgp

################################################################################
# ALL ITEMS BELOW THIS POINT ARE AUTO GENERATED BY THE BSP ASSISTANT TOOL.
# DO NOT MODIFY DIRECTLY. CHANGES SHOULD BE MADE THROUGH THE BSP ASSISTANT.
################################################################################

# Board device selection. MPN_LIST tracks what was selected in the BSP Assistant
# All other variables are derived by BSP Assistant based on the MPN_LIST.
MPN_LIST:=PSE846GPS2DBZC4A CYW55513IUBG
DEVICE:=PSE846GPS2DBZC4A
ADDITIONAL_DEVICES:=CYW55513IUBG
DEVICE_COMPONENTS:=55500 55500A1 CAT1 CAT1D PSE84
DEVICE_CYW55513IUBG_CORES:=CORE_NAME_CM33_0
DEVICE_CYW55513IUBG_DIE:=55500A1
DEVICE_CYW55513IUBG_FLASH_KB:=0
DEVICE_CYW55513IUBG_IPBLOCKS:=43XXX_BT 43XXX_BT_V1 43XXX_COEX 43XXX_COEX_V1 43XXX_WIFI 43XXX_WIFI_V1 55500A0_IOSS 55500A0_IOSS_V1 55500A0_SRSS 55500A0_SRSS_V1 BT_WIFI_ANA_PMU BT_WIFI_ANA_PMU_V1 BT_WIFI_AUDIO BT_WIFI_AUDIO_V1 BT_WIFI_DMIC BT_WIFI_DMIC_V1 BT_WIFI_I2C BT_WIFI_I2C_V1 BT_WIFI_SPI BT_WIFI_SPI_V1 BT_WIFI_UART BT_WIFI_UART_V1 H28ADCCOMP H28ADCCOMP_V1 MXSCB_VER2_S40E MXSCB_VER2_S40E_V2 MXSMIF_VER3_S40E MXSMIF_VER3_S40E_V3 MXTCPWM_VER2 MXTCPWM_VER2_V2 MXTDM MXTDM_V2
DEVICE_CYW55513IUBG_SRAM_KB:=768
DEVICE_DEFINES:=BLHS_SUPPORT TRXV5
DEVICE_LIST:=PSE846GPS2DBZC4A CYW55513IUBG
DEVICE_PSE846GPS2DBZC4A_CORES:=CORE_NAME_CM33_0 CORE_NAME_CM55_0
DEVICE_PSE846GPS2DBZC4A_DIE:=EXPLORER
DEVICE_PSE846GPS2DBZC4A_FEATURES:=CM33_0_CMSE_PRESENT CM33_0_DSP_PRESENT CM33_0_FPU_PRESENT CM55_0_DSP_PRESENT CM55_0_FPU_PRESENT CM55_0_MVE_PRESENT EPC2
DEVICE_PSE846GPS2DBZC4A_FLASH_KB:=0
DEVICE_PSE846GPS2DBZC4A_IPBLOCKS:=M0SECCPUSS M0SECCPUSS_V2 M33SYSCPUSS M33SYSCPUSS_V2 M55APPCPUSS M55APPCPUSS_V1 MXCM33 MXCM33_V2 MXCM55 MXCM55_V1 MXCRYPTO MXCRYPTO_V2 MXDEBUG600 MXDEBUG600_V1 MXDFT MXDFT_V3 MXDW MXDW_V1 MXETH MXETH_V3 MXFAULT MXFAULT_V1 MXI3C MXI3C_V1 MXIPC MXIPC_V1 MXNNLITE MXNNLITE_V2 MXPDM MXPDM_V1 MXS22GFXSS MXS22GFXSS_V1 MXS22IOSS MXS22IOSS_V1 MXS22LPCOMP MXS22LPCOMP_V1 MXS22LPPASS MXS22LPPASS_V1 MXS22RRAMC MXS22RRAMC_V1 MXS22SCB MXS22SCB_V1 MXS22SRSS MXS22SRSS_V1 MXS22USBHS MXS22USBHS_V1 MXSAXIDMAC MXSAXIDMAC_V1 MXSDHC MXSDHC_V2 MXSMIF MXSMIF_V6 MXSOCMEM MXSOCMEM_V1 MXSPERI MXSPERI_V2 MXSRAMC MXSRAMC_V2 MXTCPWM_VER2 MXTCPWM_VER2_V2 MXTDM MXTDM_V3 MXTTCANFD MXTTCANFD_V4 MXU55 MXU55_V1
DEVICE_PSE846GPS2DBZC4A_RRAM_KB:=512
DEVICE_PSE846GPS2DBZC4A_SRAM_KB:=1024
DEVICE_TOOL_IDS:=audio-fe-configurator bsp-assistant bt-configurator device-configurator dfuh-tool edge-protect-configurator library-manager ml-configurator project-creator qspi-configurator smartio-configurator
RECIPE_DIR:=$(SEARCH_recipe-make-cat1d)
