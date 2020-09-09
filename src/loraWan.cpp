/*
 * loraWan.cpp
 *
 *  Created on: 21 Jul 2019
 *      Author: uDEV - www.udev.com.br
 */

#include <loraWan.h>

#include "mbed.h"
#include "LoRaWANInterface.h"
#include "LoRaRadio.h"
#include "us_ticker_api.h"
#include "Event.h"
#include "EventQueue.h"
#include "../mbed-semtech-lora-rf-drivers/SX1272/SX1272_LoRaRadio.h"
#include "../mbed-semtech-lora-rf-drivers/SX126X/SX126X_LoRaRadio.h"

#include "Event.h"
#include "LoRaWANInterface.h"
#include "LoRaRadio.h"

#include "us_ticker_api.h"


uint8_t  NwkSKey[16] = MBED_CONF_LORA_NWKSKEY;
uint8_t  AppSKey[16] = MBED_CONF_LORA_APPSKEY;
uint8_t  NwkKey[16]  = MBED_CONF_LORA_NETWORK_KEY;
uint8_t  AppKey[16]  = MBED_CONF_LORA_APPLICATION_KEY;


class LoraWanClass
{

public:
	static LoraWanClass * instance();

private:
	LoraWanClass();
	virtual ~LoraWanClass();

public:
	int16_t send ( uint8_t port, const uint8_t *data, uint16_t length, int flags );
	int16_t receive ( uint8_t * port, uint8_t * data, uint16_t length, int * flags );

	void handle_event_queue();

private:
	static void loraWanEventsCallback ( lorawan_event_t event );
	static void loraWanCheckResponseCallback( uint8_t demod_margin, uint8_t num_gw );

private:
	events::EventQueue ev_queue;

	LoRaRadio * m_radio;

	LoRaWANInterface * m_lorawanInterface;

	lorawan_app_callbacks_t m_lorawanAppCallbacks;

};

LoraWanClass * LoraWanClass::instance()
{
	static LoraWanClass __instance;
	return &__instance;
}

LoraWanClass::LoraWanClass()
{

	lorawan_connect_t m_connection;

#if 1
	m_radio = new SX1272_LoRaRadio ( SX127X_SPI_MOSI,
									 SX127X_SPI_MISO,
									 SX127X_SPI_CLK,
									 SX127X_SPI_CS,
									 SX127X_RESET,
									 SX127X_SPI_DIO0,
									 SX127X_SPI_DIO1,
									 SX127X_SPI_DIO2,
									 SX127X_SPI_DIO3,
									 SX127X_SPI_DIO4,
									 SX127X_SPI_DIO5 );

#endif

#if 0
	m_radio = new SX126X_LoRaRadio ( SX126X_SPI_MOSI,
									SX126X_SPI_MISO,
									SX126X_SPI_CLK,
									SX126X_SPI_CS,
									SX126X_RESET,
									SX126X_DIO1,
									SX126X_BUSY,
									SX126X_FREQ_SELECT,
									SX126X_DEVICE_SELECT,
									SX126X_CRYSTAL_SELECT,
									SX126X_ANT_SWITCH );
#endif

	m_lorawanInterface = new LoRaWANInterface(*m_radio);

	m_lorawanInterface->initialize(&ev_queue);

	m_lorawanAppCallbacks.events          = mbed::callback( loraWanEventsCallback );
	m_lorawanAppCallbacks.link_check_resp = mbed::callback( loraWanCheckResponseCallback );
	m_lorawanAppCallbacks.battery_level   = NULL;

	m_lorawanInterface->add_app_callbacks( &m_lorawanAppCallbacks );

	//
	// Important!! Don't forget to modify the preprocessor symbols
	//

#if ( MBED_CONF_LORA_OVER_THE_AIR_ACTIVATION == 0 )

	m_connection.connect_type = LORAWAN_CONNECTION_ABP;
	m_connection.connection_u.abp.nwk_id = 1;
	m_connection.connection_u.abp.dev_addr = MBED_CONF_LORA_DEVICE_ADDRESS;
	m_connection.connection_u.abp.app_skey = AppSKey;
	m_connection.connection_u.abp.nwk_skey = NwkSKey;
	m_connection.connection_u.abp.nwk_senckey = NwkSKey;
	m_connection.connection_u.abp.snwk_sintkey = NwkSKey;

#elif ( MBED_CONF_LORA_OVER_THE_AIR_ACTIVATION == 1 )

	m_connection.connect_type = LORAWAN_CONNECTION_OTAA;
	m_connection.connection_u.abp.nwk_id = 1;
	m_connection.connection_u.abp.dev_addr = MBED_CONF_LORA_DEVICE_ADDRESS;
	m_connection.connection_u.abp.app_skey = AppSKey;
	m_connection.connection_u.abp.nwk_skey = NwkKey;
	m_connection.connection_u.abp.nwk_senckey = AppSKey;
	m_connection.connection_u.abp.snwk_sintkey = AppSKey;

#endif

	m_lorawanInterface->disable_adaptive_datarate();

	m_lorawanInterface->set_datarate( DR_2 );

	m_lorawanInterface->set_device_class( CLASS_A );

	m_lorawanInterface->connect( m_connection );

}

LoraWanClass::~LoraWanClass()
{
	delete m_lorawanInterface;
	delete m_radio;
}

int16_t
LoraWanClass::send ( uint8_t port, const uint8_t *data, uint16_t length, int flags )
{

	return m_lorawanInterface->send( port, data, length, flags );

}


int16_t
LoraWanClass::receive ( uint8_t * port, uint8_t * data, uint16_t length, int * flags )
{

	return m_lorawanInterface->receive( data, length, *port, *flags );
}


void
LoraWanClass::handle_event_queue(){

	ev_queue.dispatch( 10 );

}

const char * ac_lorawan_event[] = {

   "CONNECTED\r\n",
   "DISCONNECTED\r\n",
   "TX_DONE\r\n",
   "TX_TIMEOUT\r\n",
   "TX_ERROR\r\n",
   "CRYPTO_ERROR\r\n",
   "TX_SCHEDULING_ERROR\r\n",
   "RX_DONE\r\n",
   "RX_TIMEOUT\r\n",
   "RX_ERROR\r\n",
   "JOIN_FAILURE\r\n",
   "UPLINK_REQUIRED\r\n",
   "AUTOMATIC_UPLINK_ERROR\r\n",
   "CLASS_CHANGED\r\n", //only in Lorawan 1.1 (ch 18.1)
   "SERVER_ACCEPTED_CLASS_IN_USE\r\n", //only in Lorawan 1.1 (ch 18.1)
   "SERVER_DOES_NOT_SUPPORT_CLASS_IN_USE\r\n", //only in Lorawan 1.1 (ch 18.1)
   "DEVICE_TIME_SYNCHED\r\n", // only in LoRaWAN v1.0.3 and v1.1.x

};


int tx_done;
int tx_error;
int rx_done;
int rx_timeout;

uint8_t rx_data[256];
uint8_t rx_port;
int rx_flag;

void
LoraWanClass::loraWanEventsCallback ( lorawan_event_t event )
{

    switch (event) {
        case CONNECTED:
            break;
        case DISCONNECTED:
            break;
        case TX_DONE:

        	tx_done++;

            break;
        case TX_TIMEOUT:
            break;
        case TX_ERROR:

        	tx_error++;

            break;
        case CRYPTO_ERROR:
            break;
        case TX_SCHEDULING_ERROR:
            break;
        case RX_DONE:
        {
        	rx_done++;

        	LoraWanClass * instance = LoraWanClass::instance();
        	instance->receive( &rx_port, rx_data, 256, &rx_flag );
        }
            break;
        case RX_TIMEOUT:

        	rx_timeout++;

            break;
        case RX_ERROR:
            break;
        case JOIN_FAILURE:
            break;
        case UPLINK_REQUIRED:
            break;
        case AUTOMATIC_UPLINK_ERROR:
            break;
        case CLASS_CHANGED: //only in Lorawan 1.1 (ch 18.1)
            break;
        case SERVER_ACCEPTED_CLASS_IN_USE: //only in Lorawan 1.1 (ch 18.1)
            break;
        case SERVER_DOES_NOT_SUPPORT_CLASS_IN_USE: //only in Lorawan 1.1 (ch 18.1)
            break;
        case DEVICE_TIME_SYNCHED: // only in LoRaWAN v1.0.3 and v1.1.x
            break;
    }
}

void
LoraWanClass::loraWanCheckResponseCallback(uint8_t demod_margin, uint8_t num_gw)
{

// Treat Check Response here

}

extern "C" LoraWanClass* loraWanClass_instance()
{
    return LoraWanClass::instance();
}

extern "C" void loraWanClass_handle_event_queue ( LoraWanClass* loraWanClass )
{
	loraWanClass->handle_event_queue();
}

extern "C" void loraWanClass_send ( LoraWanClass* loraWanClass, uint8_t port, const uint8_t *data, uint16_t length, int flags )
{
	loraWanClass->send( port, data, length, flags );
}

extern "C" int16_t loraWanClass_rx ( LoraWanClass* loraWanClass, uint8_t * port, uint8_t * data, uint16_t length, int * flags )
{
	return loraWanClass->receive( port, data, length, flags );
}
