/**
****************************************************************************************
*
* @file  app_sps_task.c
*
* @brief SPS application Message Handlers.
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
 * @addtogroup APPTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"               // SW configuration
#include <stdint.h>

#if (BLE_APP_PRESENT)
#include "app_task.h"                  		// Application Task API
#include "app_sps_ble.h" 

#if (BLE_SPS_SERVER)
#include "sps_server_task.h"      // SPS task functions
#endif //BLE_SPS_SERVER

#if (BLE_SPS_CLIENT)
#include "sps_client_task.h"   // SPS task functions
#endif //BLE_SPS_CLIENT

#include "app_sps_uart.h"
#include "uart_sps.h"


uint16_t ble_tx_hndl __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

#if (BLE_SPS_SERVER)

/**
 ****************************************************************************************
 * @brief Handles start indication if the database is created
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_sps_create_db_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_create_db_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // If state is not idle, ignore the message
    if (ke_state_get(dest_id) == APP_DB_INIT)
    {
        // Inform the Application Manager
        struct app_module_init_cmp_evt *cfm = KE_MSG_ALLOC(APP_MODULE_INIT_CMP_EVT,
                                                           TASK_APP, TASK_APP,
                                                           app_module_init_cmp_evt);

        cfm->status = param->status;

        ke_msg_send(cfm);

    }
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles enable indication of the database
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int app_sps_server_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_server_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    
    ble_tx_hndl = param->data_hdl;
    
    return (KE_MSG_CONSUMED);
}
#endif /*(BLE_SPS_SERVER)*/
                                                                                                                                                              
#if (BLE_SPS_CLIENT)
int app_sps_client_enable_cfm_handler(ke_msg_id_t const msgid,
                                      struct sps_client_enable_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    
    ble_tx_hndl = param->sps_client.chars[SPS_SERVER_RX_DATA].val_hdl;
    
    return (KE_MSG_CONSUMED);
}
#endif /*(BLE_SPS_CLIENT)*/

#endif //(BLE_APP_PRESENT)

/// @} APPTASK

