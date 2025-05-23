# KIT_PSE84_EVAL_EPC2 BSP

## Overview





To use code from the BSP, simply include a reference to `cybsp.h`.

## Features


## BSP Configuration

The BSP has a few hooks that allow its behavior to be configured. Some of these items are enabled by default while others must be explicitly enabled. Items enabled by default are specified in the KIT_PSE84_EVAL_EPC2.mk file. The items that are enabled can be changed by creating a custom BSP or by editing the application makefile.

Components:

Defines:
* CYBSP_WIFI_CAPABLE - This define, disabled by default, causes the BSP to initialize the interface to an onboard wireless chip if it has one.
* CYBSP_CUSTOM_SYSCLK_PM_CALLBACK - This define, disabled by default, causes the BSP to skip registering its default SysClk Power Management callback, if any, and instead to invoke the application-defined function `cybsp_register_custom_sysclk_pm_callback` to register an application-specific callback.

### Clock Configuration

| Clock    | Source    | Output Frequency |
|----------|-----------|------------------|
| CLK_HF0  | CLK_PATH0 | 200 MHz          |
| CLK_HF1  | CLK_PATH0 | 399 MHz          |
| CLK_HF2  | CLK_PATH0 | 200 MHz          |
| CLK_HF3  | CLK_PATH0 | 200 MHz          |
| CLK_HF4  | CLK_PATH0 | 399 MHz          |
| CLK_HF5  | CLK_PATH0 | 200 MHz          |
| CLK_HF6  | CLK_PATH0 | 200 MHz          |
| CLK_HF7  | CLK_PATH1 | 49 MHz           |
| CLK_HF8  | CLK_PATH0 | 50 MHz           |
| CLK_HF9  | CLK_PATH0 | 80 MHz           |
| CLK_HF10 | CLK_PATH0 | 100 MHz          |
| CLK_HF11 | CLK_PATH0 | 200 MHz          |
| CLK_HF12 | CLK_PATH4 | 24 MHz           |
| CLK_HF13 | CLK_PATH0 | 100 MHz          |

### Power Configuration

* System Active Power Mode: HP
* System Idle Power Mode: Deep Sleep
* VDDA Voltage: 1800 mV
* VDDD Voltage: 1800 mV

See the [BSP Setttings][settings] for additional board specific configuration settings.

## API Reference Manual

The KIT_PSE84_EVAL_EPC2 Board Support Package provides a set of APIs to configure, initialize and use the board resources.

See the [BSP API Reference Manual][api] for the complete list of the provided interfaces.

## More information
* [KIT_PSE84_EVAL_EPC2 BSP API Reference Manual][api]
* [KIT_PSE84_EVAL_EPC2 Documentation](https://www.infineon.com/cms/en/product/evaluation-boards/placeholder/)
* [Cypress Semiconductor, an Infineon Technologies Company](http://www.cypress.com)
* [Infineon GitHub](https://github.com/infineon)
* [ModusToolbox™](https://www.cypress.com/products/modustoolbox-software-environment)

[api]: https://infineon.github.io/TARGET_KIT_PSE84_EVAL_EPC2/html/modules.html
[settings]: https://infineon.github.io/TARGET_KIT_PSE84_EVAL_EPC2/html/md_bsp_settings.html

---
© Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation, 2019-2024.