/***************************************************************************//**
 * @file analogin_api.c
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

#include "hal/analogin_api.h"
#include "hal/pinmap.h"
#include "em_cmu.h"
#include "platform/mbed_assert.h"

void analogin_init(analogin_t *obj, PinName pin) {

}

uint16_t analogin_read_u16(analogin_t *obj) {

	// Only used to check proper SX1262 configuration, we are using all the default values.
	// Change the return value if different.

	return 0;

}
