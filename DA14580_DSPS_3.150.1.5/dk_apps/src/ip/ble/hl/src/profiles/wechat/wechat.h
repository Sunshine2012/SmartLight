

#ifndef WECHAT_H_
#define WECHAT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#if (BLE_WECHAT)

#include "ke_task.h"
#include "atts.h"
#include "prf_types.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Handles offsets
enum
{
    WECHAT_1_IDX_SVC,

    WECHAT_1_IDX_CHAR,
    WECHAT_1_IDX_VAL,

    WECHAT_2_IDX_CHAR,
    WECHAT_2_IDX_VAL,
    WECHAT_2_IDX_CFG,
    
    WECHAT_1_IDX_NB,
};

///Characteristics Code for Write Indications
enum
{
    WECHAT_ERR_CHAR,
    WECHAT_1_CHAR,
    WECHAT_2_CFG,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// wechat environment variable
struct wechat_env_tag
{
    /// Connection Information
    struct prf_con_info con_info;

    /// wechat  svc Start Handle
    uint16_t wechat_shdl;
    
    //Notification property status
    uint8_t feature;

};

/*
 *  WECHAT PROFILE ATTRIBUTES VALUES DECLARATION
 ****************************************************************************************
 */

/*
/// wechat Service
extern const struct att_uuid_128 wechat_svc;
/// wechat_1 - Characteristic
extern struct att_char128_desc wechat_1_char;
/// wechat_1 - Value
extern const struct att_uuid_128 wechat_1_val;
/// wechat_2 - Characteristic
extern struct att_char128_desc wechat_2_char;
/// wechat_2 - Value
extern const struct att_uuid_128 wechat_2_val;

*/
// 修改为16位的UUID aiwesky 20151005
/// wechat Service
extern const att_size_t wechat_svc;
/// wechat_1 - Characteristic
extern struct att_char_desc wechat_1_char;
/// wechat_1 - Value
extern const att_size_t wechat_1_val;
/// wechat_2 - Characteristic
extern struct att_char_desc wechat_2_char;
/// wechat_2 - Value
extern const att_size_t wechat_2_val;



/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern struct wechat_env_tag wechat_env;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the wechat module.
 * This function performs all the initializations of the WECHAT module.
 ****************************************************************************************
 */
void wechat_init(void);

/**
 ****************************************************************************************
 * @brief Send value change to application.
 * @param val Value.
 ****************************************************************************************
 */
 
void wechat_send_val(uint8_t val);

/**
 ****************************************************************************************
 * @brief Disable role.
 ****************************************************************************************
 */
void wechat_disable(void);

/**
 ****************************************************************************************
 * @brief Send update char2 confirmation message
 ****************************************************************************************
 */
void wechat_upd_char2_cfm_send(uint8_t status);
#endif //BLE_WECHAT

#endif // WECHAT_H_

