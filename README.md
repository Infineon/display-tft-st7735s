# 0.96-inch TFT LCD display (ST7735S)

## Overview

This library provides functions to support a 0.96-inch TFT LCD driven by an ST7735S controller. This is the display used on the SHIELD_XENSIV_A shield.


## Quick start

Follow these steps to add the driver to an application for a supported kit (for example, KIT_PSE84_EVAL_EPC2 or KIT_PSE84_EVAL_EPC4).

1. Create a [PSOC&trade; Edge MCU: Empty application](https://github.com/Infineon/mtb-example-psoc-edge-empty-app) by following the "Create a new application" section in [AN235935 – Getting started with PSOC&trade; Edge E8 on ModusToolbox&trade; software](https://www.infineon.com/AN235935) application note

2. Add the *display-tft-st7735s* library to this application using Library Manager

3. The SCB SPI block (`CYBSP_SPI_CONTROLLER`) is by default enabled and configured in the `KIT_PSE84_EVAL_EPC2` and `KIT_PSE84_EVAL_EPC4` Device Configurator. Ensure it is enabled as a master, with its SCLK, MOSI, and CS pins. Because the driver sends plain 8-bit byte buffers, ensure the SPI block is configured as follows:
    - Mode: **SPI master**
    - Data width: **8-bit** (for both RX and TX)
    - Bit order: **MSB first**
    - SPI mode: **0** (CPOL = 0, CPHA = 0)
    - Clock speed: **Up to 15 MHz**

4. Place the following code in the `main.c` file. Update the D/C and Reset pin macros to match your board's wiring

```c
#include "cybsp.h"
#include "mtb_st7735s.h"

/* Panel control GPIOs (SCLK/MOSI/CS come from the SPI block). Adjust to match
 * your board's wiring. */
#define DISPLAY_DC_PORT   GPIO_PRT15
#define DISPLAY_DC_PIN    2U
#define DISPLAY_RST_PORT  GPIO_PRT13
#define DISPLAY_RST_PIN   6U

/* Infineon Ocean-500 (#0A8276) as big-endian RGB565. */
#define OCEAN_500_RGB565  0x0C0EU

static cy_stc_scb_spi_context_t spi_context;
static mtb_st7735s_t display;

int main(void)
{
    cy_rslt_t result;
    cy_en_scb_spi_status_t initStatus;

    /* Initializes the device and board peripherals. */
    result = cybsp_init();
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Enables global interrupts. */
    __enable_irq();

    /* Initializes SPI for interfacing with display. */
    initStatus = Cy_SCB_SPI_Init(CYBSP_SPI_CONTROLLER_HW,
                                 &CYBSP_SPI_CONTROLLER_config,
                                 &spi_context);
    if (CY_SCB_SPI_SUCCESS != initStatus)
    {
        CY_ASSERT(0);
    }
    Cy_SCB_SPI_Enable(CYBSP_SPI_CONTROLLER_HW);

    /* Maps the display pins to the baseboard and configure the orientation. */
    const mtb_st7735s_config_t config =
    {
        .pins =
        {
            .dc_port  = DISPLAY_DC_PORT,
            .dc_pin   = DISPLAY_DC_PIN,
            .rst_port = DISPLAY_RST_PORT,
            .rst_pin  = DISPLAY_RST_PIN,
        },
        .orientation = MTB_ST7735S_ORIENTATION_LANDSCAPE,
    };

    /* Initializes the display. */
    result = mtb_st7735s_init(&display, CYBSP_SPI_CONTROLLER_HW, &config);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Fills the entire screen with Ocean-500. */
    uint8_t row[MTB_ST7735S_PANEL_HEIGHT * 2U];

    for (uint32_t x = 0U; x < display.width; x++)
    {
        row[2U * x]      = (uint8_t)(OCEAN_500_RGB565 >> 8);
        row[2U * x + 1U] = (uint8_t)(OCEAN_500_RGB565 & 0xFFU);
    }

    mtb_st7735s_set_address_window(&display, 0U, 0U,
                                   display.width - 1U,
                                   display.height - 1U);

    for (uint32_t y = 0U; y < display.height; y++)
    {
        mtb_st7735s_write_data_stream(&display, row,
                                      (uint32_t)display.width * 2U);
    }

    for (;;)
    {
    }
}
```

5. Build the application and program the kit


## More information

For more information, refer to the following documents:

* [API reference guide](https://infineon.github.io/display-tft-st7735s/html/index.html)
* [ModusToolbox&trade; software environment, quick start guide, documentation, and videos](https://www.infineon.com/modustoolbox)
* [Infineon Technologies AG](https://www.infineon.com)

-----
© 2024-2026, Infineon Technologies AG
