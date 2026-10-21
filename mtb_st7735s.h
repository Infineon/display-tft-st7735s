/*******************************************************************************
* \file mtb_st7735s.h
*
* \brief
*    This is display ST7735S interface header file.
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

#if !defined(MTB_ST7735S_H)
#define MTB_ST7735S_H

#include <stdint.h>
#include "cy_result.h"
#include "cy_gpio.h"
#include "cy_scb_spi.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * \addtogroup group_st7735s ST7735S TFT Display Driver
 * \{
 * Driver for the ST7735S 0.96-inch 80 x 160 resolution RGB TFT panel over a 4-wire SPI
 * interface. The driver owns the full panel bring-up (reset, power/gamma/frame
 * configuration, color format, orientation) and provides windowed pixel writes.
 * It is instance based: one \ref mtb_st7735s_t object per physical panel.
 *
 * As the caller owns the SPI block, initialize and enable it before
 * calling \ref mtb_st7735s_init. Pixel data is streamed as-is; the caller is
 * responsible for supplying the panel's byte order (big-endian RGB565).
 *
 * The driver sends plain 8-bit byte buffers, so the SPI block must be
 * configured to match the ST7735S:
 * - SPI master, 8-bit data width (RX and TX)
 * - MSB first
 * - SPI mode 0 (CPOL = 0, CPHA = 0)
 * - SCLK up to 15 MHz
 *
 ********************************************************************************
 * \section group_st7735s_snippets Code snippets
 ********************************************************************************
 * * \ref subsection_st7735s_snippet_1
 * * \ref subsection_st7735s_snippet_2
 *
 ********************************************************************************
 * \subsection subsection_st7735s_snippet_1 Snippet 1: Initializing the ST7735S controller
 ********************************************************************************
 * The following snippet demonstrates how to initialize the ST7735S display over
 * a 4-wire SPI interface with the D/C and Reset GPIO configuration.
 * \snippet main.c snippet_st7735s_init
 *
 ********************************************************************************
 * \subsection subsection_st7735s_snippet_2 Snippet 2: Filling the screen
 ********************************************************************************
 * The following snippet shows how to set the address window and stream pixel
 * data to fill the whole panel with a solid color.
 * \snippet main.c snippet_st7735s_fill
 */


/** Native panel width (portrait), in pixels. The active pixel count is
 * orientation independent, so use this with \ref MTB_ST7735S_PANEL_HEIGHT to
 * size a full-frame render buffer:
 * MTB_ST7735S_PANEL_WIDTH * MTB_ST7735S_PANEL_HEIGHT * 2 bytes for RGB565. */
#define MTB_ST7735S_PANEL_WIDTH     (80U)
/** Native panel height (portrait), in pixels. See \ref MTB_ST7735S_PANEL_WIDTH. */
#define MTB_ST7735S_PANEL_HEIGHT    (160U)

/** Display orientation. Selects the MADCTL rotation/mirroring and the windowed
 * GRAM start offsets applied internally by the driver. */
typedef enum
{
    MTB_ST7735S_ORIENTATION_PORTRAIT  = 0,  /**< 80 x 160 (native). */
    MTB_ST7735S_ORIENTATION_LANDSCAPE = 1   /**< 160 x 80 (rotated 90 degrees). */
} mtb_st7735s_orientation_t;

/** GPIO pins used by the ST7735S in addition to the SPI block's MOSI/SCLK/CS. */
typedef struct
{
    GPIO_PRT_Type* dc_port;   /**< Port for the display D/C signal. */
    uint32_t       dc_pin;    /**< Pin for the display D/C signal. */
    GPIO_PRT_Type* rst_port;  /**< Port for the display Reset signal. */
    uint32_t       rst_pin;   /**< Pin for the display Reset signal. */
} mtb_st7735s_pins_t;

/** Configuration passed to \ref mtb_st7735s_init. Copied by the driver, so it
 * may be a temporary. */
typedef struct
{
    mtb_st7735s_pins_t        pins;         /**< D/C and Reset GPIOs. */
    mtb_st7735s_orientation_t orientation;  /**< Display orientation. */
} mtb_st7735s_config_t;

/** Driver instance/context. Populated by \ref mtb_st7735s_init. Treat the
 * fields as read-only; \c width and \c height reflect the active orientation. */
typedef struct
{
    CySCB_Type*        spi_base;    /**< SPI SCB base used to drive the panel. */
    mtb_st7735s_pins_t pins;        /**< Driver-owned copy of the D/C and Reset pins. */
    uint16_t           width;       /**< Active width for the current orientation. */
    uint16_t           height;      /**< Active height for the current orientation. */
    uint16_t           col_offset;  /**< Column (X) GRAM start offset. */
    uint16_t           row_offset;  /**< Row (Y) GRAM start offset. */
} mtb_st7735s_t;

/**
 * Initializes an ST7735S panel: Configures the D/C and Reset GPIOs, performs a
 * hardware reset, runs the full panel configuration sequence, applies the
 * requested orientation, turns the display on, and selects the full-screen
 * address window. The SPI block referenced by \p spi_inst must already be
 * initialized and enabled by the caller.
 * @pre \p obj, \p spi_inst and \p config must be non-NULL.
 * @pre \p config->pins.dc_port and \p config->pins.rst_port must be non-NULL.
 * @pre \p config->orientation must be a valid \ref mtb_st7735s_orientation_t value.
 * @param[out] obj        Driver instance.
 * @param[in]  spi_inst   SPI SCB base used to interface with the ST7735S.
 * @param[in]  config     Pin and orientation configuration (copied internally).
 * @return Always returns CY_RSLT_SUCCESS; the return value is reserved for future use.
 */
cy_rslt_t mtb_st7735s_init(mtb_st7735s_t* obj, CySCB_Type* spi_inst,
                           const mtb_st7735s_config_t* config);

/**
 * Sets the column (CASET) and row (RASET) address window that the subsequent
 * pixel stream is written into, applying the panel start offsets, and issues
 * RAMWR so the panel is ready to receive pixels. Coordinates are inclusive and
 * in display (post-orientation) pixels.
 * @pre \p obj must be non-NULL and initialized by \ref mtb_st7735s_init.
 * @param[in] obj    Driver instance.
 * @param[in] x1     Start column.
 * @param[in] y1     Start row.
 * @param[in] x2     End column.
 * @param[in] y2     End row.
 */
void mtb_st7735s_set_address_window(mtb_st7735s_t* obj, uint16_t x1, uint16_t y1,
                                    uint16_t x2, uint16_t y2);

/**
 * Streams a block of pixel/data bytes to the panel with D/C = 1. Typically,
 * called after \ref mtb_st7735s_set_address_window. Byte order must match the
 * panel (big-endian RGB565); any swap is the caller's responsibility.
 * @pre \p obj must be non-NULL and initialized by \ref mtb_st7735s_init.
 * @param[in] obj          Driver instance.
 * @param[in] data         Pointer to the data to send.
 * @param[in] size_bytes   Number of bytes to send.
 */
void mtb_st7735s_write_data_stream(mtb_st7735s_t* obj, const uint8_t* data,
                                   uint32_t size_bytes);

/**
 * Writes one command byte to the panel with D/C = 0.
 * @pre \p obj must be non-NULL and initialized by \ref mtb_st7735s_init.
 * @param[in] obj        Driver instance.
 * @param[in] command    Command byte to send.
 */
void mtb_st7735s_write_command(mtb_st7735s_t* obj, uint8_t command);

/**
 * Writes one data byte to the panel with D/C = 1.
 * @pre \p obj must be non-NULL and initialized by \ref mtb_st7735s_init.
 * @param[in] obj     Driver instance.
 * @param[in] data    Data byte to send.
 */
void mtb_st7735s_write_data(mtb_st7735s_t* obj, uint8_t data);

/**
 * Releases the D/C and Reset GPIOs (set to analog Hi-Z). The SPI block is owned
 * by the caller and is left untouched. No other API is valid on \p obj after
 * this call.
 * @pre \p obj must be non-NULL and initialized by \ref mtb_st7735s_init.
 * @param[in] obj    Driver instance.
 */
void mtb_st7735s_free(mtb_st7735s_t* obj);

/** \} group_st7735s */

#if defined(__cplusplus)
}
#endif

#endif /* MTB_ST7735S_H */

/* [] END OF FILE */
