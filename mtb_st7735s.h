/*******************************************************************************
* \file mtb_st7735s.h
*
* \brief
*    This is display ST7735S interface header file.
*
********************************************************************************
* \copyright
* Copyright 2024 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#pragma once

#include <stdint.h>
#include "cy_result.h"
#include "cyhal_gpio.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * \addtogroup group_board_libs TFT Display
 * \{
 * APIs for controlling the TFT display on the board.
 */

/** Structure defining the pins used to communicate with the TFT display. */
typedef struct
{
    cyhal_gpio_t dc;        /** Pin for the display D/C signal */
    cyhal_gpio_t rst;       /** Pin for the display Reset signal */
} mtb_st7735s_pins_t;

/**
 * Structure to store the commands and their data used to configure the TFT display.
 * Not required to use in the application directly.
 */
typedef struct
{
    uint8_t command;
    uint8_t data[16];
    uint8_t size;
} _mtb_st7735s_config_t;

/**
 * Configures the the ST7735S TFT display.
 * Not required to call in the application directly.
 */
void _mtb_st7735s_config(void);

/**
 * Initializes the SPI communication and the GPIO pins for the display.
 * Note: Do not free the initialized GPIO pins, these will be used through out the program
 * @param[in] spi_inst    SPI instance to be used for interfacing with the ST7735S controller.
 * @param[in] pins    Structure providing the pins to be used for the display.
 * @return CY_RSLT_SUCCESS if properly initialized, else an error indicating what went wrong.
 */
cy_rslt_t mtb_st7735s_init_spi(cyhal_spi_t* spi_inst, const mtb_st7735s_pins_t* pins);

/**
 * Writes one byte of command to the display with the DC pin set to 0.
 * @param[in] command    Command to be sent to the display
 */
void mtb_st7735s_write_command(uint8_t command);

/**
 * Writes one byte of data to the display with the DC pin set to 1.
 * @param[in] data    Data to be sent to the display
 */
void mtb_st7735s_write_data(uint8_t data);

/**
 * Writes multiple bytes of data to the display with the DC pin set to 1.
 * @param[in] data    Pointer to the data to be sent to the display
 * @param[in] num    The number of bytes in the data array to send to the display
 */
void mtb_st7735s_write_data_stream(uint8_t* data, int num);


/**
 * Free all resources used for the display interface.
 */
void mtb_st7735s_free(void);

/** \} group_board_libs */

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
