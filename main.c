/***************************************************************************//**
 * @file
 * @brief Silicon Labs Thermometer Example Application
 * This Thermometer and OTA example allows the user to measure temperature
 * using the temperature sensor on the WSTK. The values can be read with the
 * Health Thermometer reader on the Blue Gecko smartphone app.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/* Board Headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* GATT database */
#include "gatt_db.h"

/* EM library (EMlib) */
#include "em_system.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"
#include "retargetserial.h"

#ifdef FEATURE_BOARD_DETECTED
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#else
#error This sample app only works with a Silicon Labs Board
#endif

#include "bsp.h"

#include "i2cspm.h"
#include "si7013.h"
#include "tempsens.h"
#include "loraWan.h"


/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

LoraWanClass * lorawanclass;
uint8_t lora_tx_counter = 0;
#define LORA_TX_COUNTER_IN_SEC	5

/* Gecko configuration parameters (see gecko_configuration.h) */
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 1
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

static const gecko_configuration_t config = {
  .config_flags = 0,
#if defined(FEATURE_LFXO)
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
#else
  .sleep.flags = 0,
#endif // LFXO
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE)
  .pa.config_enable = 1, // Set this to be a valid PA config
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#else
  .pa.input = GECKO_RADIO_PA_INPUT_DCDC,
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
#endif // (HAL_PA_ENABLE)
  .rf.flags = GECKO_RF_CONFIG_ANTENNA,                 /* Enable antenna configuration. */
  .rf.antenna = GECKO_RF_ANTENNA,                      /* Select antenna path! */
};

/* Flag for indicating DFU Reset must be performed */
uint8_t boot_to_dfu = 0;

/**
 * @brief Function for taking a single temperature measurement with the WSTK Relative Humidity and Temperature (RHT) sensor.
 */
void temperatureMeasure()
{

  uint8_t lorawanPayload[8];
  int32_t loraRawTempData = 0;
  uint32_t loraRawRhData = 0;

  uint8_t htmTempBuffer[5]; /* Stores the temperature data in the Health Thermometer (HTM) format. */
  uint8_t flags = 0x00;   /* HTM flags set as 0 for Celsius, no time stamp and no temperature type. */
  int32_t tempData;     /* Stores the Temperature data read from the RHT sensor. */
  uint32_t rhData = 0;    /* Dummy needed for storing Relative Humidity data. */
  uint32_t temperature;   /* Stores the temperature data read from the sensor in the correct format */
  uint8_t *p = htmTempBuffer; /* Pointer to HTM temperature buffer needed for converting values to bitstream. */
  static int32_t DummyValue = 0l;

  /* Convert flags to bitstream and append them in the HTM temperature data buffer (htmTempBuffer) */
  UINT8_TO_BITSTREAM(p, flags);

  /* Sensor relative humidity and temperature measurement returns 0 on success, nonzero otherwise */
  if (Si7013_MeasureRHAndTemp(I2C0, SI7021_ADDR, &rhData, &tempData) != 0) {
    tempData = DummyValue + 20000l;
    DummyValue = (DummyValue + 1000l) % 21000l;
  }

  loraRawTempData = tempData;
  loraRawRhData = rhData;

  /* Convert sensor data to correct temperature format */
  temperature = FLT_TO_UINT32(tempData, -3);
  /* Convert temperature to bitstream and place it in the HTM temperature data buffer (htmTempBuffer) */
  UINT32_TO_BITSTREAM(p, temperature);

  /* Send indication of the temperature in htmTempBuffer to all "listening" clients.
   * This enables the Health Thermometer in the Blue Gecko app to display the temperature.
   *  0xFF as connection ID will send indications to all connections. */
  gecko_cmd_gatt_server_send_characteristic_notification(
    0xFF, gattdb_temperature_measurement, 5, htmTempBuffer);

  BSP_LedToggle(0);

  lora_tx_counter++;

  if ( lora_tx_counter >= LORA_TX_COUNTER_IN_SEC ) {

	lora_tx_counter = 0;

    memcpy( &lorawanPayload[0], (const void*)&loraRawTempData, 4 );
    memcpy( &lorawanPayload[4], (const void*)&loraRawRhData, 4 );

    // Send unconfirmed message
    loraWanClass_send( lorawanclass, 1, lorawanPayload, 8, 1 );

    // Send confirmed message
    //loraWanClass_send( lorawanclass, 1, lorawanPayload, 8, 2 );

  }


// Use the following code to decode the packet on TTN - Payload Formats
//
//	function Decoder(bytes, port) {
//
//	    var result = {tempC: "", rH: ""};
//
//	    result.tempC = ((bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0])/1000.0;
//
//	    result.rH = ((bytes[7] << 24) | (bytes[6] << 16) | (bytes[5] << 8) | bytes[4])/1000.0;
//
//	    return result;
//
//	}

}


/**
 * @brief  Main function
 */
int main(void)
{
  // Initialize device
  initMcu();

  // Initialize board
  initBoard();

  // Disable all spi peripherals on dev kit
  GPIO_PinModeSet( gpioPortA, 4, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 14, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 15, gpioModePushPull, 1 );

#if ( HAL_VCOM_ENABLE == 1 )
  // Config stdio to Virtual COM port through USB
  RETARGET_SerialInit();
#endif

  // Initialize LEDs
  BSP_LedsInit();

  // Initialize LoraWan
  lorawanclass = loraWanClass_instance();

  // Initialize application
  initApp();

  // Initialize stack
  gecko_init(&config);

  // Initialize the Temperature Sensor
  Si7013_Detect(I2C0, SI7021_ADDR, NULL);

  while (1) {

    /* Check for lorawan stack event. */
    loraWanClass_handle_event_queue( lorawanclass );

    /* Event pointer for handling events */
    struct gecko_cmd_packet* evt;

    /* Check for ble stack event. */
	
    // TODO: Check how to manage low power with BLE & Lora
    //evt = gecko_wait_event();
    evt = gecko_peek_event();

    /* Handle events */
    switch (BGLIB_MSG_ID(evt->header)) {
      /* This boot event is generated when the system boots up after reset.
       * Do not call any stack commands before receiving the boot event.
       * Here the system is set to start advertising immediately after boot procedure. */
      case gecko_evt_system_boot_id:
        /* Set advertising parameters. 100ms advertisement interval.
         * The first two parameters are minimum and maximum advertising interval, both in
         * units of (milliseconds * 1.6). */
        gecko_cmd_le_gap_set_advertise_timing(0, 160, 160, 0, 0);

        /* Start general advertising and enable connections. */
        gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
        break;

      /* This event is generated when a connected client has either
       * 1) changed a Characteristic Client Configuration, meaning that they have enabled
       * or disabled Notifications or Indications, or
       * 2) sent a confirmation upon a successful reception of the indication. */
      case gecko_evt_gatt_server_characteristic_status_id:
        /* Check that the characteristic in question is temperature - its ID is defined
         * in gatt.xml as "temperature_measurement". Also check that status_flags = 1, meaning that
         * the characteristic client configuration was changed (notifications or indications
         * enabled or disabled). */
        if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement)
            && (evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01)) {
          if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02) {
            /* Indications have been turned ON - start the repeating timer. The 1st parameter '32768'
             * tells the timer to run for 1 second (32.768 kHz oscillator), the 2nd parameter is
             * the timer handle and the 3rd parameter '0' tells the timer to repeat continuously until
             * stopped manually.*/
            gecko_cmd_hardware_set_soft_timer(32768, 0, 0);
            BSP_LedClear(0);
            BSP_LedSet(1);
            lora_tx_counter = LORA_TX_COUNTER_IN_SEC;
          } else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00) {
            /* Indications have been turned OFF - stop the timer. */
            gecko_cmd_hardware_set_soft_timer(0, 0, 0);
          }
        }
        break;

      /* This event is generated when the software timer has ticked. In this example the temperature
       * is read after every 1 second and then the indication of that is sent to the listening client. */
      case gecko_evt_hardware_soft_timer_id:
        /* Measure the temperature as defined in the function temperatureMeasure() */
        temperatureMeasure();
        break;

      case gecko_evt_le_connection_closed_id:

    	  BSP_LedClear(0);
    	  BSP_LedClear(1);

        /* Check if need to boot to dfu mode */
        if (boot_to_dfu) {
          /* Enter to DFU OTA mode */
          gecko_cmd_system_reset(2);
        } else {
          /* Stop timer in case client disconnected before indications were turned off */
          gecko_cmd_hardware_set_soft_timer(0, 0, 0);
          /* Restart advertising after client has disconnected */
          gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
        }
        break;

      /* Events related to OTA upgrading
         ----------------------------------------------------------------------------- */

      /* Checks if the user-type OTA Control Characteristic was written.
       * If written, boots the device into Device Firmware Upgrade (DFU) mode. */
      case gecko_evt_gatt_server_user_write_request_id:
        if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
          /* Set flag to enter to OTA mode */
          boot_to_dfu = 1;
          /* Send response to Write Request */
          gecko_cmd_gatt_server_send_user_write_response(
            evt->data.evt_gatt_server_user_write_request.connection,
            gattdb_ota_control,
            bg_err_success);

          /* Close connection to enter to DFU OTA mode */
          gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
        }
        break;

      default:
        break;
    }
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
