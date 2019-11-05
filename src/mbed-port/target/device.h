#ifndef SRC_DEVICE_H_
#define SRC_DEVICE_H_

#define AES_PRESENT			1
#define AES_COUNT			1
#define DEVICE_INTERRUPTIN 	1
#define DEVICE_SERIAL		2
#define DEVICE_SPI_COUNT 	2
#define DEVICE_SPI 			2
#define DEVICE_SPI_ASYNCH 	1
#define DEVICE_USTICKER 	1
#define DEVICE_ANALOGIN		1

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_pcnt.h"

#include <target/PinNames.h>
#include <target/objects.h>
#include <target/device_peripherals.h>

#endif /* SRC_DEVICE_H_ */
