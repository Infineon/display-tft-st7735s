# Display 0.96 Inch TFT LCD (ST7735S) API Reference Guide

## General Description

Basic set of API for interacting with the TFT display. This library provides functions for supporting a 0.96 inch TFT LCD driven by an ST7735S controller.

For more information about the TFT display, see: Display: https://www.winstar.com.tw/products/tft-lcd/ips-tft/mini-tft-lcd.html

**Note:** This library is intended to work with emWin middleware and currently supports a single instance of the display.

## Code Snippets

### Snippet 1: Simple initialization with SPI
The following snippet initializes the TFT display with the emWin graphics library using SPI interface.

```
/* Initialize the TFT display */
rslt = mtb_st7735s_init_spi(&spi_inst, &data);

/* If TFT init failed. Stop program execution */
CY_ASSERT(CY_RSLT_SUCCESS == rslt);

/* Initialize emWin GUI */
GUI_Init();

/* Set font size, background color and text mode */
GUI_SetFont(GUI_FONT_16_1);
#define FONT_SIZE 16
GUI_SetBkColor(GUI_WHITE);
GUI_SetColor(GUI_BLACK);
GUI_SetTextMode(GUI_TM_NORMAL);

/* Clear the display */
GUI_Clear();
```

### Snippet 2: Simple text alignment with emWin
The following snippet demonstrates how to use the emWin graphics library to align and print strings to the display.
```
/* Update display */
GUI_SetTextAlign(GUI_TA_HCENTER);
GUI_DispStringAt("TFT DISPLAY", LCD_GetXSize() / 2,(LCD_GetYSize() / 2) - (FONT_SIZE + (FONT_SIZE / 2)));
GUI_SetTextAlign(GUI_TA_HCENTER);
GUI_DispStringAt("ST7735S Controller", LCD_GetXSize() / 2, (LCD_GetYSize() / 2) - (FONT_SIZE / 2));
GUI_SetTextAlign(GUI_TA_HCENTER);
GUI_DispStringAt("emWin Example", LCD_GetXSize() / 2, (LCD_GetYSize() / 2) + (FONT_SIZE / 2));
```

## Data Structure
```
mtb_st7735s_pins_t         Structure defining the pins used to communicate with the TFT display
```

```
_mtb_st7735s_config_t      Internal structure to store the commands and their data used to configure the TFT display
```

## Functions
void `_mtb_st7735s_config(void)`
>Configures the the ST7735S TFT display. Not required to call in the application directly.

cy_rslt_t `mtb_st7735s_init_spi(cyhal_spi_t* spi_inst, const mtb_st7735s_pins_t* pins)`
>Initializes the SPI communication and the GPIO pins for the display.

void `mtb_st7735s_write_command(uint8_t command)`
>Writes one byte of command to the display with the DC pin set to 0.

void `mtb_st7735s_write_data(uint8_t data)`
>Writes one byte of data to the display with the DC pin set to 1.

void `mtb_st7735s_write_data_stream(uint8_t* data, int num)`
>Writes multiple bytes of data to the display with the DC pin set to 1.

void `mtb_st7735s_free(void)`
>Free all resources used for the ST7735 controller.

## Data Structure Documentation
- mtb_st7735s_pins_t

Data Fields       |    object      |           Description
 :-------         |  :------------ |  :------------
 cyhal_gpio_t     |    dc          | Pin for the display D/C signal
 cyhal_gpio_t     |    rst         | Pin for the display Reset signal

 - _mtb_st7735s_config_t

Data Fields  |    object      |           Description
 :-------    |  :------------ |  :------------
 uint8_t     |    command     | Command required of display configuration
 uint8_t     |    data        | An array with data required of display configuration
 uint8_t     |    size        | Size of the data

 **Note:** `_mtb_st7735s_config_t` This is an internal structure, not required to use in the application directly.

## Function Documentation
#### _mtb_st7735s_config
- void `_mtb_st7735s_config(void)`

> **Summary:** Configures the the st7735s TFT display and not required to call in the application directly.

#### mtb_st7735s_init_spi
- cy_rslt_t `mtb_st7735s_init_spi(cyhal_spi_t* spi_inst, const mtb_st7735s_pins_t* pins)`

> **Summary:** Initializes the SPI communication and the GPIO pins for the display.
>
> **Parameter:**
>  Parameters            |  Description
>  :-------              |  :------------
>  spi_inst              |  SPI instance to be used for interfacing with the ST7735S controller.
>  pins                  |  Structure providing the pins to be used for the display control.
>
> Return:
>  - cy_rslt_t           :  CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.

#### mtb_st7735s_write_command
- void `mtb_st7735s_write_command(uint8_t command)`

> **Summary:** Writes one byte of command to the display with the DC pin set to 0.
>
> **Parameter:**
>  Parameters            |  Description
>  :-------              |  :------------
>  command               |  The command to be sent to the display

#### mtb_st7735s_write_data
- void `mtb_st7735s_write_data(uint8_t data)`

> **Summary:** Writes one byte of data to the display with the DC pin set to 1.
>
> **Parameter:**
>  Parameters            |  Description
>  :-------              |  :------------
>  data                  |  Data to be sent to the display

#### mtb_st7735s_write_data_stream
- void `mtb_st7735s_write_data_stream(uint8_t* data, int num)`

> **Summary:** Writes multiple bytes of data to the display with the DC pin set to 1.
>
> **Parameter:**
>  Parameters           |  Description
>  :-------             |  :------------
>  data                 |  Pointer to the data to be sent to the display.
>  num                  |  The number of bytes in the data array to send to the display.

#### mtb_st7735s_free
- void `mtb_st7735s_free(void)`
>**Summary:** Free all resources used for the display interface.

---
© 2024, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
