 /**
 ****************************************************************************************
 *
 * @file  app_sps_ble.c
 *
 * @brief SPS application.
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>                  				// string manipulation and functions

#include "rwip_config.h"               			// SW configuration

#if (BLE_APP_PRESENT)
#include "app.h"                     				// application definitions
#include "app_sps_ble.h"
#include "app_sps_uart.h"
#if (BLE_SPS_SERVER)
#include "sps_server_task.h"       // SPS functions
#endif
#if (BLE_SPS_CLIENT)
#include "sps_client_task.h"    // SPS functions
#endif

/* test variables */
long debug_ble_nb_bytes_send = 0;
/* end */

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
#if (BLE_SPS_SERVER)
/**
 ****************************************************************************************
 * @brief 			app_sps_create_db creates a database for the sps server service
 * @param[in] 	void
 * @return 			none
 ****************************************************************************************
 */
void app_sps_create_db(void)
{						
		// Add sps server service to the database
		struct sps_server_create_db_req * req = KE_MSG_ALLOC(SPS_SERVER_CREATE_DB_REQ, TASK_SPS_SERVER,
																									 TASK_APP, sps_server_create_db_req);
		req->start_hdl = 0;

		// Send the message
		ke_msg_send(req);
}
#endif
/**
 ****************************************************************************************
 * @brief 			app_sps_init initialize the required functions
 * @param[in] 	void
 * @return 			none
 ****************************************************************************************
 */
void app_sps_init(void)
{
		// SPS application initialisation
		app_buffer_init();
		app_uart_init();
		app_flowcontrol_init();
}

/**
 ****************************************************************************************
 * @brief 		Function used to enable server role
 * @param[in] void
 * @return 		none
 ****************************************************************************************
 */
#if (BLE_SPS_SERVER)
static void app_sps_server_enable(void)
{
		// Allocate the message
		struct sps_server_enable_req * req = KE_MSG_ALLOC(SPS_SERVER_ENABLE_REQ, TASK_SPS_SERVER, TASK_APP,
																								 sps_server_enable_req);

		// Fill in the parameter structure
		req->appid = TASK_APP;
		req->conhdl = app_env.conhdl;

		// Send the message
		ke_msg_send(req);
}
#endif

/**
 ****************************************************************************************
 * @brief 		Function used to enable client role
 * @param[in] void
 * @return 		none
 ****************************************************************************************
 */
#if (BLE_SPS_CLIENT)
static void app_sps_client_enable(void)
{
		// Allocate the message
		struct sps_client_enable_req * req = KE_MSG_ALLOC(SPS_CLIENT_ENABLE_REQ, TASK_SPS_CLIENT, TASK_APP,
																							 sps_client_enable_req);
		
		// Fill in the parameter structure
		req->conhdl = app_env.conhdl;
		req->con_type = PRF_CON_DISCOVERY; 
		
		// Send the message
		ke_msg_send(req);	
}
#endif


/**
 ****************************************************************************************
 * @brief 			app_sps_enable(void) Sends enable message(s) when the connection is made.
 * @param[in] 	void
 * @return 			none
 ****************************************************************************************
 */
void app_sps_enable(void)
{
#if (BLE_SPS_SERVER) // enable the sps device so it can send notification message
		app_sps_server_enable();
#endif
		
#if (BLE_SPS_CLIENT) //enable sps host to set the device in discovery mode to search for the proper handles
		app_sps_client_enable();
#endif
}

/**
 ****************************************************************************************
 * @brief 			Send flow control over Bluetooth
 * @param[in] 	flow control 		(UART_XON or UART_XOFF)
 * @return 			none
 ****************************************************************************************
 */
#if (BLE_SPS_SERVER)
void sendFlowControlOverBluetooth(uint8_t flowcontrol)
{
		struct sps_server_send_notify_flow_control_state_req * req = KE_MSG_ALLOC(SPS_SERVER_SEND_FLOW_CONTROL_REQ, TASK_SPS_SERVER, TASK_APP,
																									sps_server_send_notify_flow_control_state_req);

			
		req->flow_control_state = (uint8_t) flowcontrol;

		ke_msg_send(req);
}
#endif // (BLE_SPS_SERVER)

#if (BLE_SPS_CLIENT)
void sendFlowControlOverBluetooth(uint8_t flowcontrol)
{

		struct sps_client_send_notify_flow_control_state_req * req = KE_MSG_ALLOC(SPS_CLIENT_SEND_FLOW_CONTROL_REQ, TASK_SPS_CLIENT, TASK_APP,
																									sps_client_send_notify_flow_control_state_req);

		req->flow_control_state = (uint8_t) flowcontrol;
           
		ke_msg_send(req);
}
#endif // (BLE_SPS_CLIENT)

#endif // (BLE_APP_PRESENT)

/// @} APP
