/**
 ****************************************************************************************
 *
 * @file app_wechat.c
 *
 * @brief // Device Information Service Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_WECHAT_SERVER)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_wechat.h"                 //
#include "app_wechat_task.h"            //
#include "app.h"                        // Application Definitions
#include "app_task.h"                   // Application Task Definitions
#include "wechat_task.h"                // 


/*
 * LOCAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void app_wechat_init(void)
{
    
    return;
}


void app_wechat_create_db_send(void)
{
    // Add WECHAT in the database
    struct wechat_create_db_req *req = KE_MSG_ALLOC(WECHAT_CREATE_DB_REQ,
                                                  TASK_WECHAT, TASK_APP,
                                                  wechat_create_db_req);

    req->features = APP_WECHAT_FEATURES;
    // Send the message
    ke_msg_send(req);
}

void app_wechat_enable_prf(uint16_t conhdl)
{
    // Allocate the message
    struct wechat_enable_req *req = KE_MSG_ALLOC(WECHAT_ENABLE_REQ,
                                               TASK_WECHAT, TASK_APP,
                                               wechat_enable_req);

    // Fill in the parameter structure
    req->conhdl             = conhdl;
    req->sec_lvl            = PERM(SVC, ENABLE);
    //req->con_type           = PRF_CON_DISCOVERY;  // aiwesky 20151007 zhushi

    // aiwesky 20151007
    req->wechat_1_val        = 0xaa;
    req->wechat_2_val        = 0x55;
    // Send the message
    ke_msg_send(req);

}

#endif //BLE_WECHAT_SERVER

/// @} APP
