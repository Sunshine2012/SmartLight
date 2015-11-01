/**
****************************************************************************************
*
* @file app_sps_proj_task.h
*
* @brief SPS application
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

#ifndef APP_SPS_PROJ_TASK_H_
#define APP_SPS_PROJ_TASK_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "ke_msg.h"
#if (BLE_SPS_SERVER)
#include "sps_server_task.h"
#endif
#if (BLE_SPS_CLIENT)
#include "sps_client_task.h"
#endif
#include "l2cc_task.h"

#if (BLE_SPS_SERVER)

int app_sps_create_db_cfm_handler(ke_msg_id_t const msgid,
                                    struct sps_server_create_db_cfm const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);
                                    
int app_sps_server_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
                                      
                                     				                                 
#endif // (BLE_SPS_SERVER)


#if (BLE_SPS_CLIENT)
int app_sps_client_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);
#endif

/// @} APP

#endif // PRJ1_PROJ_TASK
