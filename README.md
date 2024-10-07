# Display 0.96 Inch TFT LCD (ST7735S)

## Overview

This library provides functions for supporting a 0.96 inch TFT LCD driven by an ST7735S controller. This is the display used on the SHIELD_XENSIV_A shield.

## Quick Start

Follow the steps bellow in order to create a simple emWin application and display some text on it.

1. Create an empty ModusToolbox™ application.
2. Add this (display-tft-st7735s) library to the application.
3. Add emWin library using the Library Manager.
4. Enable EMWIN_NOSNTS emWin library option by adding it to the Makefile COMPONENTS list.

```
COMPONENTS+=EMWIN_NOSNTS
```

5. Place the following code in the main.c file.

```
#include "cyhal.h"
#include "cybsp.h"
#include "mtb_st7735s.h"
#include "GUI.h"

/* GPIO pins for SPI interface */
#define TFT_SPI_MOSI (CYBSP_SPI_MOSI)
#define TFT_SPI_MISO (CYBSP_SPI_MISO)
#define TFT_SPI_CLK  (CYBSP_SPI_CLK)
#define TFT_SPI_CS   (CYBSP_SPI_CS)

/* SPI baud rate in Hz */
#define SPI_FREQ_HZ                (1000000UL)
/* SPI transfer bits per frame */
#define BITS_PER_FRAME             (8)

cyhal_spi_t spi;

/* Map the display pins to the baseboard */
const mtb_st7735s_pins_t tft_pins =
{
    .dc       = CYBSP_A0,
    .rst      = CYBSP_D2
};

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    /* Initialize display settings */
    result = mtb_st7735s_init_spi(&spi, &tft_pins);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Initialize SPI for interfacing with display */
    result = cyhal_spi_init(&spi, TFT_SPI_MOSI, TFT_SPI_MISO, TFT_SPI_CLK,
                            TFT_SPI_CS, NULL, BITS_PER_FRAME,
                            CYHAL_SPI_MODE_00_MSB, false);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }
    result = cyhal_spi_set_frequency(&spi, SPI_FREQ_HZ);
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Initialize the GUI component and show text */
    GUI_Init();
    GUI_DispString("Hello world!");

    for (;;)
    {
    }
}
```

6. Build the application and program the kit.

## More information

For more information, refer to the following documents:

* [API Reference Guide](./api_reference.md)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software)
* [Infineon Technologies AG](https://www.infineon.com)

-----
© 2024, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
