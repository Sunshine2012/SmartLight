/**
 ****************************************************************************************
 *
 * @file app_wechat_task.h
 *
 * @brief Header file - APPWECHATTASK.
 *
 * Copyright (C) RivieraWaves 2009-2013
 *
 *
 ****************************************************************************************
 */

#ifndef APP_WECHAT_TASK_H_
#define APP_WECHAT_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup APPWECHATTASK Task
 * @ingroup APPWECHAT
 * @brief Device Information Service Application Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_WECHAT_SERVER)

#include "wechat_task.h"
#include "ke_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximal number of APP DIS Task instances
#define APP_WECHAT_IDX_MAX        (1)

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles DIS Server profile database creation confirmation.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int wechat_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct wechat_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles disable indication from the WECHAT Server profile.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int wechat_disable_ind_handler(ke_msg_id_t const msgid,
                                    struct wechat_disable_ind const *param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id);                                      


#endif //(BLE_WECHAT_SERVER)

/// @} APPWECHATTASK

#endif //APP_WECHAT_TASK_H_
