/*******************************************************************************
* \file mtb_st7735s.c
*
* \brief
*    This is display ST7735S interface source file.
*
********************************************************************************
* \copyright
* Copyright 2024-2026 Cypress Semiconductor Corporation
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

#include <stddef.h>
#include "mtb_st7735s.h"
#include "cy_syslib.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* D/C line levels: 0 selects command, 1 selects data. */
#define DC_COMMAND          (0U)
#define DC_DATA             (1U)

/* D/C setup time honored before each SPI transfer, in microseconds. */
#define DC_SETUP_DELAY_US   (10U)

/* ST7735S command set (subset used by this driver). */
#define ST7735S_CMD_SLPOUT  (0x11U)  /* Sleep out. */
#define ST7735S_CMD_INVON   (0x21U)  /* Display inversion on. */
#define ST7735S_CMD_CASET   (0x2AU)  /* Column address set. */
#define ST7735S_CMD_RASET   (0x2BU)  /* Row address set. */
#define ST7735S_CMD_RAMWR   (0x2CU)  /* Memory write. */
#define ST7735S_CMD_DISPON  (0x29U)  /* Display on. */
#define ST7735S_CMD_MADCTL  (0x36U)  /* Memory data access control. */
#define ST7735S_CMD_COLMOD  (0x3AU)  /* Interface pixel format. */

/* MADCTL rotation/mirroring value per orientation (MY/MX/MV | BGR). */
#define ST7735S_MADCTL_PORTRAIT     (0xC8U)  /* MY | MX | BGR */
#define ST7735S_MADCTL_LANDSCAPE    (0xA8U)  /* MY | MV | BGR */

/* Windowed GRAM start offsets for this 0.96-inch 80 x 160 resolution module. */
#define ST7735S_PORTRAIT_COL_OFFSET   (26U)
#define ST7735S_PORTRAIT_ROW_OFFSET   (1U)
#define ST7735S_LANDSCAPE_COL_OFFSET  (1U)
#define ST7735S_LANDSCAPE_ROW_OFFSET  (26U)

/* Command + payload record used by the panel configuration table. */
typedef struct
{
    uint8_t command;
    uint8_t data[16];
    uint8_t size;
} _st7735s_cmd_t;

static void _st7735s_spi_write(mtb_st7735s_t* obj, const uint8_t* data, uint32_t num_bytes);
static void _st7735s_run_config(mtb_st7735s_t* obj);


/*******************************************************************************
* _st7735s_spi_write
*******************************************************************************/
static void _st7735s_spi_write(mtb_st7735s_t* obj, const uint8_t* data, uint32_t num_bytes)
{
    Cy_SCB_SPI_ClearTxFifo(obj->spi_base);
    Cy_SCB_SPI_ClearRxFifo(obj->spi_base);

    Cy_SCB_SPI_WriteArrayBlocking(obj->spi_base, (void*)data, num_bytes);

    /* Waits until the last data element leaves the shifter. */
    while (!Cy_SCB_SPI_IsTxComplete(obj->spi_base))
    {
    }

    /* Discards the dummy bytes clocked in during the write-only transfer. */
    Cy_SCB_SPI_ClearRxFifo(obj->spi_base);
}


/*******************************************************************************
* mtb_st7735s_write_command
*******************************************************************************/
void mtb_st7735s_write_command(mtb_st7735s_t* obj, uint8_t command)
{
    CY_ASSERT(NULL != obj);

    Cy_GPIO_Write(obj->pins.dc_port, obj->pins.dc_pin, DC_COMMAND);
    Cy_SysLib_DelayUs(DC_SETUP_DELAY_US);

    _st7735s_spi_write(obj, &command, 1UL);
}


/*******************************************************************************
* mtb_st7735s_write_data
*******************************************************************************/
void mtb_st7735s_write_data(mtb_st7735s_t* obj, uint8_t data)
{
    CY_ASSERT(NULL != obj);

    Cy_GPIO_Write(obj->pins.dc_port, obj->pins.dc_pin, DC_DATA);
    Cy_SysLib_DelayUs(DC_SETUP_DELAY_US);

    _st7735s_spi_write(obj, &data, 1UL);
}


/*******************************************************************************
* mtb_st7735s_write_data_stream
*******************************************************************************/
void mtb_st7735s_write_data_stream(mtb_st7735s_t* obj, const uint8_t* data, uint32_t size_bytes)
{
    CY_ASSERT(NULL != obj);

    Cy_GPIO_Write(obj->pins.dc_port, obj->pins.dc_pin, DC_DATA);
    Cy_SysLib_DelayUs(DC_SETUP_DELAY_US);

    _st7735s_spi_write(obj, data, size_bytes);
}


/*******************************************************************************
* _st7735s_run_config
*******************************************************************************/
static void _st7735s_run_config(mtb_st7735s_t* obj)
{
    /* Panel-specific frame-rate, power, and gamma tuning. Orientation (MADCTL)
     * and the address window (CASET/RASET) are applied separately. */
    static const _st7735s_cmd_t config[] =
    {
        /* Frame rate in normal (full-screen) mode */
        {
            .command = 0xB1,
            .data =
            {
                0x05, 0x3C, 0x3C
            },
            .size = 3
        },
        /* Frame rate in idle (8-color) mode. */
        {
            .command = 0xB2,
            .data =
            {
                0x05, 0x3C, 0x3C
            },
            .size = 3
        },
        /* Frame rate in partial mode (dot and column inversion). */
        {
            .command = 0xB3,
            .data =
            {
                0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C
            },
            .size = 6
        },
        /* Displays inversion control per display mode. */
        {
            .command = 0xB4,
            .data =
            {
                0x07
            },
            .size = 1
        },
        /* Power control 1: Panel driving voltages (GVDD, AVDD). */
        {
            .command = 0xC0,
            .data =
            {
                0xE9, 0x09, 0x04
            },
            .size = 3
        },
        /* Power control 2: Gate driving voltages (VGH, VGL). */
        {
            .command = 0xC1,
            .data =
            {
                0xC5
            },
            .size = 1
        },
        /* Power control 3: Operating current in normal mode. */
        {
            .command = 0xC2,
            .data =
            {
                0x0D, 0x00
            },
            .size = 2
        },
        /* Power control 4: Operating current in idle mode. */
        {
            .command = 0xC3,
            .data =
            {
                0x8D, 0x6A
            },
            .size = 2
        },
        /* Power control 5: Operating current in partial mode. */
        {
            .command = 0xC4,
            .data =
            {
                0x8D, 0xEE
            },
            .size = 2
        },
        /* VCOM voltage level (affects contrast and flicker). */
        {
            .command = 0xC5,
            .data =
            {
                0x15
            },
            .size = 1
        },
        /* Enables display color inversion. */
        {
            .command = ST7735S_CMD_INVON,
            .data =
            {
                0
            },
            .size = 0
        },
        /* Positive-polarity gamma curve correction. */
        {
            .command = 0xE0,
            .data =
            {
                0x07, 0x0E, 0x08, 0x07, 0x10, 0x07, 0x02, 0x07, 0x09,
                0x0F, 0x25, 0x36, 0x00, 0x08, 0x04, 0x10
            },
            .size = 16
        },
        /* Negative-polarity gamma curve correction. */
        {
            .command = 0xE1,
            .data =
            {
                0x0A, 0x0D, 0x08, 0x07, 0x0F, 0x07, 0x02, 0x07, 0x09,
                0x0F, 0x25, 0x35, 0x00, 0x09, 0x04, 0x10
            },
            .size = 16
        },
        /* Gate pump clock/power-save configuration. */
        {
            .command = 0xFC,
            .data =
            {
                0xC0
            },
            .size = 1
        },
        /* Interface pixel format: 16 bits per pixel (RGB565). */
        {
            .command = ST7735S_CMD_COLMOD,
            .data =
            {
                0x05
            },
            .size = 1
        },
    };

    /* Hardware reset pulse. */
    Cy_GPIO_Write(obj->pins.rst_port, obj->pins.rst_pin, 1UL);
    Cy_SysLib_Delay(10);
    Cy_GPIO_Write(obj->pins.rst_port, obj->pins.rst_pin, 0UL);
    Cy_SysLib_Delay(50);
    Cy_GPIO_Write(obj->pins.rst_port, obj->pins.rst_pin, 1UL);
    Cy_SysLib_Delay(100);

    /* Exit Sleep mode. */
    mtb_st7735s_write_command(obj, ST7735S_CMD_SLPOUT);
    Cy_SysLib_Delay(120);

    for (uint32_t i = 0; i < (sizeof(config) / sizeof(config[0])); i++)
    {
        mtb_st7735s_write_command(obj, config[i].command);
        for (uint32_t j = 0; j < config[i].size; j++)
        {
            mtb_st7735s_write_data(obj, config[i].data[j]);
        }
    }
}


/*******************************************************************************
* mtb_st7735s_init
*******************************************************************************/
cy_rslt_t mtb_st7735s_init(mtb_st7735s_t* obj, CySCB_Type* spi_inst,
                           const mtb_st7735s_config_t* config)
{
    CY_ASSERT((NULL != obj) && (NULL != spi_inst) && (NULL != config));
    CY_ASSERT((NULL != config->pins.dc_port) && (NULL != config->pins.rst_port));
    CY_ASSERT((MTB_ST7735S_ORIENTATION_PORTRAIT == config->orientation) ||
              (MTB_ST7735S_ORIENTATION_LANDSCAPE == config->orientation));

    obj->spi_base = spi_inst;
    obj->pins     = config->pins;   /* Stored by value: No caller lifetime dependency. */

    uint8_t madctl;
    if (MTB_ST7735S_ORIENTATION_LANDSCAPE == config->orientation)
    {
        obj->width      = MTB_ST7735S_PANEL_HEIGHT;
        obj->height     = MTB_ST7735S_PANEL_WIDTH;
        obj->col_offset = ST7735S_LANDSCAPE_COL_OFFSET;
        obj->row_offset = ST7735S_LANDSCAPE_ROW_OFFSET;
        madctl          = ST7735S_MADCTL_LANDSCAPE;
    }
    else
    {
        obj->width      = MTB_ST7735S_PANEL_WIDTH;
        obj->height     = MTB_ST7735S_PANEL_HEIGHT;
        obj->col_offset = ST7735S_PORTRAIT_COL_OFFSET;
        obj->row_offset = ST7735S_PORTRAIT_ROW_OFFSET;
        madctl          = ST7735S_MADCTL_PORTRAIT;
    }

    /* D/C driven LOW (command), reset driven HIGH (inactive). */
    Cy_GPIO_Pin_FastInit(obj->pins.dc_port, obj->pins.dc_pin, CY_GPIO_DM_STRONG_IN_OFF, 0UL,
                         HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(obj->pins.rst_port, obj->pins.rst_pin, CY_GPIO_DM_STRONG_IN_OFF, 1UL,
                         HSIOM_SEL_GPIO);

    /* Reset and base panel configuration. */
    _st7735s_run_config(obj);

    /* Orientation-specific memory access control. */
    mtb_st7735s_write_command(obj, ST7735S_CMD_MADCTL);
    mtb_st7735s_write_data(obj, madctl);

    /* Display on. */
    mtb_st7735s_write_command(obj, ST7735S_CMD_DISPON);
    Cy_SysLib_Delay(120);

    /* Selects the full-screen window and leave the panel ready for RAMWR. */
    mtb_st7735s_set_address_window(obj, 0U, 0U,
                                   (uint16_t)(obj->width - 1U), (uint16_t)(obj->height - 1U));

    return CY_RSLT_SUCCESS;
}


/*******************************************************************************
* mtb_st7735s_set_address_window
*******************************************************************************/
void mtb_st7735s_set_address_window(mtb_st7735s_t* obj, uint16_t x1, uint16_t y1,
                                    uint16_t x2, uint16_t y2)
{
    CY_ASSERT(NULL != obj);

    x1 = (uint16_t)(x1 + obj->col_offset);
    x2 = (uint16_t)(x2 + obj->col_offset);
    y1 = (uint16_t)(y1 + obj->row_offset);
    y2 = (uint16_t)(y2 + obj->row_offset);

    mtb_st7735s_write_command(obj, ST7735S_CMD_CASET);
    mtb_st7735s_write_data(obj, (uint8_t)(x1 >> 8));
    mtb_st7735s_write_data(obj, (uint8_t)(x1 & 0xFFU));
    mtb_st7735s_write_data(obj, (uint8_t)(x2 >> 8));
    mtb_st7735s_write_data(obj, (uint8_t)(x2 & 0xFFU));

    mtb_st7735s_write_command(obj, ST7735S_CMD_RASET);
    mtb_st7735s_write_data(obj, (uint8_t)(y1 >> 8));
    mtb_st7735s_write_data(obj, (uint8_t)(y1 & 0xFFU));
    mtb_st7735s_write_data(obj, (uint8_t)(y2 >> 8));
    mtb_st7735s_write_data(obj, (uint8_t)(y2 & 0xFFU));

    /* RAMWR resets the RAM write pointer to the window origin. */
    mtb_st7735s_write_command(obj, ST7735S_CMD_RAMWR);
}


/*******************************************************************************
* mtb_st7735s_free
*******************************************************************************/
void mtb_st7735s_free(mtb_st7735s_t* obj)
{
    CY_ASSERT(NULL != obj);

    Cy_GPIO_SetDrivemode(obj->pins.dc_port, obj->pins.dc_pin, CY_GPIO_DM_ANALOG);
    Cy_GPIO_SetDrivemode(obj->pins.rst_port, obj->pins.rst_pin, CY_GPIO_DM_ANALOG);
    obj->spi_base = NULL;
}


#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
