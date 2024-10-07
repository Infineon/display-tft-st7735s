/*******************************************************************************
* \file mtb_st7735s.c
*
* \brief
*    This is display ST7735S interface source file.
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

#include "mtb_st7735s.h"
#include "cyhal.h"
#include "cybsp.h"

#if defined(__cplusplus)
extern "C"
{
#endif


#define DC_ON    (1u)
#define DC_OFF   (0u)
#define DELAY_US (10U)

static cyhal_spi_t* spi_ptr = NULL;
static const mtb_st7735s_pins_t* pins;


/*******************************************************************************
* _mtb_st7735s_config
*******************************************************************************/
void _mtb_st7735s_config(void)
{
    /* Store commands and their data in ST7735S configuration structure*/
    static _mtb_st7735s_config_t config[] =
    {
        { .command = 0xB1, .data = { 0x05, 0x3C, 0x3C }, .size = 3 },
        { .command = 0xB2, .data = { 0x05, 0x3C, 0x3C }, .size = 3 },
        { .command = 0xB3, .data = { 0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}, .size = 6 },
        { .command = 0xB4, .data = { 0x07 },.size = 1 },
        { .command = 0xC0, .data = { 0xE9, 0x09, 0x04 }, .size = 3 },
        { .command = 0xC1, .data = { 0xC5 },.size = 1 },
        { .command = 0xC2, .data = { 0x0D, 0x00 },.size = 2 },
        { .command = 0xC3, .data = { 0x8D, 0x6A },.size = 2 },
        { .command = 0xC4, .data = { 0x8D, 0xEE },.size = 2 },
        { .command = 0xC5, .data = { 0x15 },.size = 1 },
        { .command = 0x36, .data = { 0xC8 },.size = 1 },
        { .command = 0x21, .data = { 0x00 },.size = 0 },
        { .command = 0xE0, .data = { 0x07, 0x0E, 0x08, 0x07, 0x10, 0x07, 0x02, 0x07, 0x09,
                                     0x0F, 0x25, 0x36, 0x00, 0x08, 0x04, 0x10 }, .size = 16 },
        { .command = 0xE1, .data = { 0x0A, 0x0D, 0x08, 0x07, 0x0F, 0x07, 0x02, 0x07, 0x09,
                                     0x0F, 0x25, 0x35, 0x00, 0x09, 0x04, 0x10 }, .size = 16 },
        { .command = 0xFC, .data = { 0xC0 },.size = 1 },
        { .command = 0x3A, .data = { 0x05 },.size = 1 },
        { .command = 0x2A, .data = { 0x00, 0x1A, 0x00, 0x69}, .size = 4 },
        { .command = 0x2B, .data = { 0x00, 0x01, 0x00, 0xA0}, .size = 4 },
    };

    /* Reset the display controller */
    cyhal_gpio_write(pins->rst, 1);
    cyhal_system_delay_ms(10);
    cyhal_gpio_write(pins->rst, 0);
    cyhal_system_delay_ms(50);
    cyhal_gpio_write(pins->rst, 1);
    cyhal_system_delay_ms(100);

    /* Exit Sleep mode */
    mtb_st7735s_write_command(0x11);
    cyhal_system_delay_ms(120);

    uint8_t num_cmds =  sizeof(config) / sizeof(config[0]);

    // Write the commands and their data using the config array
    for (int i = 0; i < num_cmds; i++)
    {
        mtb_st7735s_write_command(config[i].command);
        for (int j = 0; j < config[i].size; j++)
        {
            mtb_st7735s_write_data(config[i].data[j]);
        }
    }

    /* Display on */
    mtb_st7735s_write_command(0X29);
    mtb_st7735s_write_command(0x2C);
    cyhal_system_delay_ms(120);
}


/*******************************************************************************
* mtb_st7735s_write_command
*******************************************************************************/
cy_rslt_t mtb_st7735s_init_spi(cyhal_spi_t* spi_inst, const mtb_st7735s_pins_t* data)
{
    cy_rslt_t rslt;
    CY_ASSERT(spi_inst != NULL);
    spi_ptr = spi_inst;
    pins = data;

    rslt = cyhal_gpio_init(pins->dc, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0u);
    if (CY_RSLT_SUCCESS == rslt)
    {
        rslt = cyhal_gpio_init(pins->rst, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1u);
        if (CY_RSLT_SUCCESS != rslt)
        {
            cyhal_gpio_free(pins->dc);
        }
    }
    return rslt;
}


/*******************************************************************************
* mtb_st7735s_write_command
*******************************************************************************/
void mtb_st7735s_write_command(uint8_t data)
{
    cy_rslt_t rslt;
    cyhal_gpio_write(pins->dc, DC_OFF);
    cyhal_system_delay_us(DELAY_US);

    rslt = cyhal_spi_send(spi_ptr, data);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);
    (void)rslt;
}


/*******************************************************************************
* mtb_st7735s_write_data
*******************************************************************************/
void mtb_st7735s_write_data(uint8_t data)
{
    cy_rslt_t rslt;
    cyhal_gpio_write(pins->dc, DC_ON);
    cyhal_system_delay_us(DELAY_US);

    rslt = cyhal_spi_send(spi_ptr, data);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);
    (void)rslt;
}


/*******************************************************************************
* mtb_st7735s_write_data_stream
*******************************************************************************/
void mtb_st7735s_write_data_stream(uint8_t* data, int num_bytes)
{
    cy_rslt_t rslt;

    cyhal_gpio_write(pins->dc, DC_ON);
    cyhal_system_delay_us(DELAY_US);

    rslt = cyhal_spi_transfer(spi_ptr, (const uint8_t*)data, num_bytes, NULL, 0, 0);
    CY_ASSERT(CY_RSLT_SUCCESS == rslt);
    (void)rslt;
}


/*******************************************************************************
* mtb_st7735s_free
*******************************************************************************/
void mtb_st7735s_free(void)
{
    cyhal_gpio_free(pins->dc);
    cyhal_gpio_free(pins->rst);
    spi_ptr = NULL;
}


#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
