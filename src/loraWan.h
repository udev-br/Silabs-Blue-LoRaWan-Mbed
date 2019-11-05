/*
 * loraWan.h
 *
 *  Created on: 21 Jul 2019
 *      Author: uDEV - www.udev.com.br
 */

#ifndef LORAWAN_H_
#define LORAWAN_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef struct LoraWanClass LoraWanClass;


LoraWanClass* loraWanClass_instance();

void loraWanClass_handle_event_queue ( LoraWanClass* loraWanClass );
void loraWanClass_send(LoraWanClass* loraWanClass , uint8_t port, const uint8_t *data, uint16_t length, int flags );


#ifdef __cplusplus
}
#endif

#endif /* LORAWAN_H_ */
