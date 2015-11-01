/**
 ****************************************************************************************
 *
 * @file sps_server_task.h
 *
 * @brief Serial Port Service Device profile task declaration
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef SPS_SERVER_TASK_H_
#define SPS_SERVER_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SPS_SERVERTASK Serial Port Service Device Task
 * @ingroup SPS_SERVER
 * @brief Serial Port Service Device task implementation
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "ke_task.h"


/*
 * INSTANCES
 ****************************************************************************************
 */
/// Maximum number of instances of the SPS_SERVER task
#define SPS_SERVER_IDX_MAX 1

#define SPS_TX_PACKET_SIZE 60

/*
 * STATES
 ****************************************************************************************
 */
/// Possible states of the SPS_SERVER task
enum
{
    /// DISABLED state
    SPS_SERVER_DISABLED,
    /// IDLE state
    SPS_SERVER_IDLE,
    /// ACTIVE state
    SPS_SERVER_ACTIVE,
    /// Number of states.
    SPS_SERVER_STATE_MAX
};

/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the SPS_SERVER task
enum
{
    /// Enables the Serial Port Service Device profile. The profile has to be enabled only
    /// once a connection has been established by the application
    SPS_SERVER_ENABLE_REQ = KE_FIRST_MSG(TASK_SPS_SERVER),
    
    //Enable condirmation message
    SPS_SERVER_ENABLE_CFM,
    
		/// Create SPS database request
    SPS_SERVER_CREATE_DB_REQ,
    
		/// Create SPS database response
    SPS_SERVER_CREATE_DB_CFM,
		
		// Send flow control status
		SPS_SERVER_SEND_FLOW_CONTROL_REQ,
	
		// send data over bluetooth
		SPS_SEND_DATA_PACKETS_REQ,
};

/// @ref SPS_SERVER_ENABLE_REQ parameters structure description.
struct sps_server_enable_req
{
    /// Application Task Id
    ke_task_id_t appid;
    
		/// Connection handle
    uint16_t conhdl;
};

struct sps_server_enable_cfm
{
		/// Connection handle
    uint16_t data_hdl;
};

///Parameters of the @ref SPS_SERVER_CREATE_DB_REQ message
struct sps_server_create_db_req
{
    ///SPS Service start handle (0 = automatic handle allocation)
    uint16_t start_hdl;
};

///Parameters of the @ref SPS_SERVER_CREATE_DB_CFM message
struct sps_server_create_db_cfm
{
    ///Status
    uint8_t status;
};

/// @ref SPS_SERVER_SEND_FLOW_CONTROL_REQ parameters structure description,
struct sps_server_send_notify_flow_control_state_req
{
		// flow control state
		uint8_t 	flow_control_state;
};

/// @ref SPS_SERVER_SEND_FLOW_CONTROL_REQ parameters structure description,
struct sps_send_data_packets_req
{
    uint8_t 	packet[SPS_TX_PACKET_SIZE];
		uint8_t 	packet_size;
};

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
extern const struct ke_state_handler sps_server_state_handler[SPS_SERVER_STATE_MAX];
extern const struct ke_state_handler sps_server_default_handler;
extern ke_state_t sps_server_state[SPS_SERVER_IDX_MAX];


/// @} SPS_SERVERTASK

#endif // SPS_SERVER_TASK_H_
