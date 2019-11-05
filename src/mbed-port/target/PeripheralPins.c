/***************************************************************************//**
 * @file PeripheralPins.c
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/


#include "mbed_critical.h"
#include "mbed_power_mgmt.h"
#include "mbed_wait_api.h"
#include "pinmap.h"
#include "mbedtls/platform.h"

#include "em_core.h"

#include "features/mbedtls/inc/mbedtls/cipher.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>


const PinMap PinMap_SPI_MOSI[] = {
#ifdef USART0_BASE
    /* USART0 */
    {PA0,  SPI_0,  0},
    {PA1,  SPI_0,  1},
    {PA2,  SPI_0,  2},
    {PA3,  SPI_0,  3},
    {PA4,  SPI_0,  4},
    {PA5,  SPI_0,  5},
    {PB11, SPI_0,  6},
    {PB12, SPI_0,  7},
    {PB13, SPI_0,  8},
    {PB14, SPI_0,  9},
    {PB15, SPI_0, 10},
    {PD9,  SPI_0, 17},
    {PD10, SPI_0, 18},
    {PD11, SPI_0, 19},
    {PD12, SPI_0, 20},
    {PD13, SPI_0, 21},
    {PD14, SPI_0, 22},
    {PD15, SPI_0, 23},
#endif
#ifdef USART1_BASE
    /* USART1 */
    {PC6,  SPI_1, 11},
    {PC7,  SPI_1, 12},
    {PC8,  SPI_1, 13},
    {PC9,  SPI_1, 14},
    {PC10, SPI_1, 15},
    {PC11, SPI_1, 16},
    {PF0,  SPI_1, 24},
    {PF1,  SPI_1, 25},
    {PF2,  SPI_1, 26},
    {PF3,  SPI_1, 27},
    {PF4,  SPI_1, 28},
    {PF5,  SPI_1, 29},
    {PF6,  SPI_1, 30},
    {PF7,  SPI_1, 31},
#endif
    {NC  , NC   , NC}

};

const PinMap PinMap_SPI_MISO[] = {
#ifdef USART0_BASE
    /* USART0 */
    {PA0,  SPI_0, 31},
    {PA1,  SPI_0,  0},
    {PA2,  SPI_0,  1},
    {PA3,  SPI_0,  2},
    {PA4,  SPI_0,  3},
    {PA5,  SPI_0,  4},
    {PB11, SPI_0,  5},
    {PB12, SPI_0,  6},
    {PB13, SPI_0,  7},
    {PB14, SPI_0,  8},
    {PB15, SPI_0,  9},
    {PD9,  SPI_0, 16},
    {PD10, SPI_0, 17},
    {PD11, SPI_0, 18},
    {PD12, SPI_0, 19},
    {PD13, SPI_0, 20},
    {PD14, SPI_0, 21},
    {PD15, SPI_0, 22},
#endif
#ifdef USART1_BASE
    /* USART1 */
    {PC6,  SPI_1, 10},
    {PC7,  SPI_1, 11},
    {PC8,  SPI_1, 12},
    {PC9,  SPI_1, 13},
    {PC10, SPI_1, 14},
    {PC11, SPI_1, 15},
    {PF0,  SPI_1, 23},
    {PF1,  SPI_1, 24},
    {PF2,  SPI_1, 25},
    {PF3,  SPI_1, 26},
    {PF4,  SPI_1, 27},
    {PF5,  SPI_1, 28},
    {PF6,  SPI_1, 29},
    {PF7,  SPI_1, 30},
    {PA0,  SPI_1, 31},
#endif
    {NC  , NC   , NC}

};


const PinMap PinMap_SPI_CLK[] = {
#ifdef USART0_BASE
   /* USART0 */
	{PA0,  SPI_0, 30},
	{PA1,  SPI_0, 31},
	{PA2,  SPI_0,  0},
	{PA3,  SPI_0,  1},
	{PA4,  SPI_0,  2},
	{PA5,  SPI_0,  3},
	{PB11, SPI_0,  4},
	{PB12, SPI_0,  5},
	{PB13, SPI_0,  6},
	{PB14, SPI_0,  7},
	{PB15, SPI_0,  8},
	{PD9,  SPI_0, 15},
	{PD10, SPI_0, 16},
	{PD11, SPI_0, 17},
	{PD12, SPI_0, 18},
	{PD13, SPI_0, 19},
	{PD14, SPI_0, 20},
	{PD15, SPI_0, 21},
#endif
#ifdef USART1_BASE
	/* USART1 */
	{PC6,  SPI_1,  9},
	{PC7,  SPI_1, 10},
	{PC8,  SPI_1, 11},
	{PC9,  SPI_1, 12},
	{PC10, SPI_1, 13},
	{PC11, SPI_1, 14},
	{PF0,  SPI_1, 22},
	{PF1,  SPI_1, 23},
	{PF2,  SPI_1, 24},
	{PF3,  SPI_1, 25},
	{PF4,  SPI_1, 26},
	{PF5,  SPI_1, 27},
	{PF6,  SPI_1, 28},
	{PF7,  SPI_1, 29},
	{PA0,  SPI_1, 30},
	{PA1,  SPI_1, 31},
#endif
	{NC  , NC   , NC}
};

const PinMap PinMap_SPI_CS[] = {
#ifdef USART0_BASE
    /* USART0 */
    {PA0,  SPI_0,  29},
    {PA1,  SPI_0,  30},
    {PA2,  SPI_0,  31},
    {PA3,  SPI_0,  0},
    {PA4,  SPI_0,  1},
    {PA5,  SPI_0,  2},
    {PB11, SPI_0,  3},
    {PB12, SPI_0,  4},
    {PB13, SPI_0,  5},
    {PB14, SPI_0,  6},
    {PB15, SPI_0,  7},
    {PD9,  SPI_0, 14},
    {PD10, SPI_0, 15},
    {PD11, SPI_0, 16},
    {PD12, SPI_0, 17},
    {PD13, SPI_0, 18},
    {PD14, SPI_0, 19},
    {PD15, SPI_0, 20},
#endif
#ifdef USART1_BASE
    /* USART1 */
    {PC6,  SPI_1,  8},
    {PC7,  SPI_1,  9},
    {PC8,  SPI_1, 10},
    {PC9,  SPI_1, 11},
    {PC10, SPI_1, 12},
    {PC11, SPI_1, 13},
    {PF0,  SPI_1, 21},
    {PF1,  SPI_1, 22},
    {PF2,  SPI_1, 23},
    {PF3,  SPI_1, 24},
    {PF4,  SPI_1, 25},
    {PF5,  SPI_1, 26},
    {PF6,  SPI_1, 27},
    {PF7,  SPI_1, 28},
#endif
    {NC  , NC   , NC}

};
