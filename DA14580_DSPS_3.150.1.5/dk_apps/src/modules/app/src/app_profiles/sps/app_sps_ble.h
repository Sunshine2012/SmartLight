/**
 ****************************************************************************************
 *
 * @file app_audio.h
 *
 * @brief SPS Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef APP_SPS_BLE_H_
#define APP_SPS_BLE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwble_config.h"
#include <stdint.h>          // standard integer definition
#include <co_bt.h>
#if (BLE_SPS_SERVER)
#include "sps_server.h"
#endif
#if (BLE_SPS_CLIENT)
#include "sps_client.h"
#endif

/*
 * STRUCTURES
 ****************************************************************************************
 */
typedef struct {
	uint8_t 	*data[BLE_MAX_PACKET_COUNT];
	uint8_t 	size[BLE_MAX_PACKET_COUNT];
	uint8_t 	packet_count;
}ble_transmit_packet;

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
 
extern uint8_t audio_adv_count;
extern uint16_t audio_adv_interval;
extern ble_transmit_packet *ble_transmit_data; 


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void app_sps_create_db(void);

/**
 ****************************************************************************************
 * @brief Initialize sps application
 ****************************************************************************************
 */
void app_sps_init(void);
 
/**
 ****************************************************************************************
 * @brief Enable the the notifcation of the SPS or enable the host to set it in discovery mode
 ****************************************************************************************
 */
void app_sps_enable(void);

/**
 ****************************************************************************************
 * @brief Send flow control state over Bluetooth
 ****************************************************************************************
*/
void sendFlowControlOverBluetooth(uint8_t flowcontrol);

/**
 ****************************************************************************************
 * @brief send data over Bluetooth
 ****************************************************************************************
*/
//uint8_t sendDataOverBluetooth(void);

/// @} APP

#endif // APP_SPS_BLE_H_
