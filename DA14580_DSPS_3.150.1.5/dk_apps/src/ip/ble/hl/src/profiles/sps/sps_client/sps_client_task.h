/**
 ****************************************************************************************
 *
 * @file sps_client_task.h
 *
 * @brief Header file - SPS_CLIENTTASK.
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: 5767 $
 *
 ****************************************************************************************
 */

#ifndef SPS_CLIENT_TASK_H_
#define SPS_CLIENT_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SPS_CLIENTTASK StreamData client Task
 * @ingroup SPS_CLIENT
 * @brief StreamData Host Task
 *
 * The StreamData client Task is responsible for handling the API messages received from
 * either the Application or internal tasks.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "ke_task.h"
#include "co_error.h"
#include "sps_client.h"
#if (BLE_SPS_SERVER)
#include "sps_server.h"
#endif

#if (BLE_SPS_CLIENT)
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of StreamData client task instances
#define SPS_CLIENT_IDX_MAX    	(BLE_CONNECTION_MAX)
#define SPS_CLIENT_PACKET_SIZE 	(BLE_MAX_PACKET_SIZE) 
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the SPS_CLIENT task
enum
{
    /// IDLE state
    SPS_CLIENT_IDLE,
    
		///Connected state
    SPS_CLIENT_CONNECTED,
    
		///Discovering
    SPS_CLIENT_DISCOVERING,
    
		/// Number of defined states.
    SPS_CLIENT_STATE_MAX
};

///StreamData Host API messages
enum
{
    ///StreamData client role enable request from application.
    SPS_CLIENT_ENABLE_REQ = KE_FIRST_MSG(TASK_SPS_CLIENT),
    
		/// StreamData Host role enable confirmation to application.
		SPS_CLIENT_ENABLE_CFM,

		/// This function will send data
		SPS_CLIENT_SEND_DATA_PACKETS_REQ,
    
		/// Inform APP that the profile client role has been disabled after a disconnection
    SPS_CLIENT_DISABLE_IND,
    
    SPS_CLIENT_SEND_FLOW_CONTROL_REQ,

};

/*
 * API Messages Structures
 ****************************************************************************************
 */

/// StreamData client enable command structure
struct sps_client_enable_req
{
    /// Connection handle
    uint16_t conhdl;
    
		/// Connection type
    uint8_t con_type;
    
		/// StreamData Device details kept in APP
    struct sps_client_sps_content sps_server;
};

/// StreamData client enable confirm structure
struct sps_client_enable_cfm
{
    /// Connection handle
    uint16_t conhdl;
   
		/// Status
    uint8_t status;
    
		/// StreamData Device details to keep in APP
    struct sps_client_sps_content sps_client;
};

struct sps_client_send_data_packets_req
{
		// Number of packages
		uint8_t 	nr_packets;
		
		// Data value
    uint16_t 	packets[SPS_CLIENT_MAX][SPS_CLIENT_PACKET_SIZE/2];
		
		// Value size
		uint8_t 	packets_size[SPS_CLIENT_MAX];
};

/// @ref SPS_SERVER_SEND_FLOW_CONTROL_REQ parameters structure description,
struct sps_client_send_notify_flow_control_state_req
{
		// flow control state
		uint8_t 	flow_control_state;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler sps_client_state_handler[SPS_CLIENT_STATE_MAX];
extern const struct ke_state_handler sps_client_default_handler;
extern ke_state_t sps_client_state[SPS_CLIENT_IDX_MAX];

/*
 * Functions
 ****************************************************************************************
 */



#endif //BLE_SPS_CLIENT

/// @} SPS_CLIENTTASK

#endif // SPS_CLIENT_TASK_H_
