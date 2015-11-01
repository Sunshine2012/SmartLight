/**
 ****************************************************************************************
 *
 * @file wechat_task.h
 *
 * @brief Header file - wechat_task.
 * @brief 128 UUID service. sample code
 *
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef WECHAT_TASK_H_
#define WECHAT_TASK_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
*/

#if (BLE_WECHAT_SERVER)

#include <stdint.h>
#include "ke_task.h"
#include "wechat.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of wechat task instances
#define WECHAT_IDX_MAX                 (1)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the WECHAT task
enum
{
    /// Disabled State
    WECHAT_DISABLED,
    /// Idle state
    WECHAT_IDLE,
    /// Connected state
    WECHAT_CONNECTED,

    /// Number of defined states.
    WECHAT_STATE_MAX
};

/// Messages for wechat
enum
{
    /// Start wechat. Device connection
    WECHAT_ENABLE_REQ = KE_FIRST_MSG(TASK_WECHAT),

    /// Disable confirm.
    WECHAT_DISABLE_IND,

    /// Att Value change indication
    WECHAT_VAL_IND,

    ///Create DataBase
    WECHAT_CREATE_DB_REQ,
    ///Inform APP of database creation status
    WECHAT_CREATE_DB_CFM,

    ///Create DataBase
    WECHAT_UPD_CHAR2_REQ,
    ///Inform APP of database creation status
    WECHAT_UPD_CHAR2_CFM,
    
    /// Error Indication
    WECHAT_ERROR_IND,
};


/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref WECHAT_ENABLE_REQ message
struct wechat_enable_req
{
    /// Connection Handle
    uint16_t conhdl;
    /// Security level
    uint8_t sec_lvl;

    /// characteristic 1 value
    uint8_t wechat_1_val;
    
    /// characteristic 2 value
    uint8_t wechat_2_val;
    
    /// char 2 Ntf property status
    uint8_t feature;
};

/// Parameters of the @ref WECHAT_DISABLE_IND message
struct wechat_disable_ind
{
    /// Connection Handle
    uint16_t conhdl;
};

/// Parameters of the @ref WECHAT_VAL_IND message
struct wechat_val_ind
{
    /// Connection handle
    uint16_t conhdl;
    /// Value
    uint8_t val;
    
};

/// Parameters of the @ref WECHAT_CREATE_DB_REQ message
struct wechat_create_db_req
{
    /// Indicate if TXPS is supported or not
    uint8_t features;
};

/// Parameters of the @ref WECHAT_CREATE_DB_CFM message
struct wechat_create_db_cfm
{
    /// Status
    uint8_t status;
};

/// Parameters of the @ref WECHAT_UPD_CHAR2_REQ message
struct wechat_upd_char2_req
{
    /// Connection handle
    uint16_t conhdl;
    /// Characteristic Value
    uint8_t val;
};

/// Parameters of the @ref WECHAT_UPD_CHAR2_CFM message
struct wechat_upd_char2_cfm
{
    /// Status
    uint8_t status;
};

/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler wechat_state_handler[WECHAT_STATE_MAX];
extern const struct ke_state_handler wechat_default_handler;
extern ke_state_t wechat_state[WECHAT_IDX_MAX];

#endif //BLE_WECHAT

#endif // WECHAT_TASK_H_


