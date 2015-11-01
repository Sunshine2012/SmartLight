/**
 ****************************************************************************************
 *
 * @file wechat.c
 *
 * @brief 128 UUID service. Sample code
 *
 * Copyright (C) 2013 Dialog Semiconductor GmbH and its Affiliates, unpublished work
 * This computer program includes Confidential, Proprietary Information and is a Trade Secret 
 * of Dialog Semiconductor GmbH and its Affiliates. All use, disclosure, and/or 
 * reproduction is prohibited unless authorized in writing. All Rights Reserved.
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (BLE_WECHAT)

#include "wechat.h"
#include "wechat_task.h"
#include "attm_db.h"
#include "gapc.h"
/*
 *  WECHAT PROFILE ATTRIBUTES VALUES DEFINTION
 ****************************************************************************************
 */

/// wechat_1 Service
const att_size_t wechat_svc     = 0xFEE7;



/// wechat_1 value attribute UUID
const att_size_t wechat_1_val     = 0xFEC7;


struct att_char128_desc wechat_1_char = {ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    {0,0},
                                                                    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}}; 

const att_size_t wechat_2_val     = 0xFEC9;

struct att_char128_desc wechat_2_char = {ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF,
                                                                    {0,0},
                                                                    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F}}; 



/*
/// wechat_1 value attribute UUID
const struct att_uuid_128 wechat_1_val     = {{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}};


struct att_char128_desc wechat_1_char = {ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR,
                                                                    {0,0},
                                                                    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F}}; 

const struct att_uuid_128 wechat_2_val     = {{0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F}};

struct att_char128_desc wechat_2_char = {ATT_CHAR_PROP_RD | ATT_CHAR_PROP_NTF,
                                                                    {0,0},
                                                                    {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F}}; 

*/
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

struct wechat_env_tag wechat_env __attribute__((section("retention_mem_area0"),zero_init));

static const struct ke_task_desc TASK_DESC_WECHAT = {wechat_state_handler, &wechat_default_handler, wechat_state, WECHAT_STATE_MAX, WECHAT_IDX_MAX};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void wechat_init(void)
{
    // Reset Environment
    memset(&wechat_env, 0, sizeof(wechat_env));
    
    // Create WECHAT task
    ke_task_create(TASK_WECHAT, &TASK_DESC_WECHAT);

    ke_state_set(TASK_WECHAT, WECHAT_DISABLED);
}

void wechat_send_val(uint8_t val)
{
    // Allocate the alert value change indication
   struct wechat_val_ind *ind = KE_MSG_ALLOC(WECHAT_VAL_IND,
                                              wechat_env.con_info.appid, TASK_WECHAT,
                                              wechat_val_ind);
   // Fill in the parameter structure
   ind->conhdl = gapc_get_conhdl(wechat_env.con_info.conidx);
   ind->val = val;
   
   // Send the message
   ke_msg_send(ind);
}

void wechat_disable(void)
{
    att_size_t length;
    uint8_t *alert_lvl;

    // Disable service in database
    attmdb_svc_set_permission(wechat_env.wechat_shdl, PERM_RIGHT_DISABLE);

    struct wechat_disable_ind *ind = KE_MSG_ALLOC(WECHAT_DISABLE_IND,
                                                 wechat_env.con_info.appid, TASK_WECHAT,
                                                 wechat_disable_ind);

    //Get value stored in DB
    attmdb_att_get_value(wechat_env.wechat_shdl + WECHAT_1_IDX_VAL,
                         &length, &alert_lvl);

    // Fill in the parameter structure
    ind->conhdl     = gapc_get_conhdl(wechat_env.con_info.conidx);

    // Send the message
    ke_msg_send(ind);

    // Go to idle state
    ke_state_set(TASK_WECHAT, WECHAT_IDLE);
}


void wechat_upd_char2_cfm_send(uint8_t status)
{
    struct wechat_upd_char2_cfm *cfm = KE_MSG_ALLOC(WECHAT_UPD_CHAR2_CFM,
                                                 wechat_env.con_info.appid, TASK_WECHAT,
                                                 wechat_upd_char2_cfm);

    cfm->status = status;
    
    // Send the message
    ke_msg_send(cfm);

}
#endif //BLE_WECHAT


