/**
****************************************************************************************
*
* @file app_sps_proj.h
*
* @brief SPS Project application header file.
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

#ifndef APP_SPS_PROJ_H_
#define APP_SPS_PROJ_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief SPS Application entry point.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"
#include "app_task.h"                  	// application task
#include "gapc_task.h"                  // gap functions and messages
#include "gapm_task.h"                  // gap functions and messages
#include "app.h"                       	// application definitions
#include "co_error.h"                  	// error code definitions
#include "smpc_task.h"                  // error code definitions
#include "app_sps_proj_task.h"

// aiwesky 20151004 添加dis_server
#if (BLE_DIS_SERVER)
#include "app_dis.h"
#include "app_dis_task.h"
#endif

// aiwesky 20151005 添加电池服务
#if (BLE_BATT_SERVER)
#include "app_batt.h"
#include "app_batt_task.h"
#endif

// aiwesky 20151007 添加空中更新服务
#if (BLE_SPOTA_RECEIVER)
#include "app_spotar.h"
#include "app_spotar_task.h"
#endif

// aiwesky 20151007 添加微信服务
#if (BLE_WECHAT_SERVER)
#include "app_wechat.h"
#include "app_wechat_task.h"
#endif 
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
 
 /****************************************************************************
Define device name. Used in Advertise string
*****************************************************************************/
#if BLE_SPS_SERVER
#define APP_DFLT_DEVICE_NAME "DA14580 SPS SmartLight"
#else
#define APP_DFLT_DEVICE_NAME "DA14580 SmartLight"
#endif



/**
 * Default Advertising data
 * --------------------------------------------------------------------------------------
 * x02 - Length
 * x01 - Flags
 * x06 - LE General Discoverable Mode + BR/EDR Not Supported
 * --------------------------------------------------------------------------------------
 * x03 - Length
 * x03 - Complete list of 16-bit UUIDs available
 * x09\x18 - Health Thermometer Service UUID
 *   or
 * x00\xFF - Nebulization Service UUID
 * --------------------------------------------------------------------------------------
 */

//#define APP_DFLT_ADV_DATA        "\x03\x03\xE7\xFE\x1B\x01\x00\x00\xCA\xEA"
#define APP_DFLT_ADV_DATA        "\x03\x03\xE7\xFE"
#define APP_DFLT_ADV_DATA_LEN    (4)

/**
 * Default Scan response data
 * --------------------------------------------------------------------------------------
 * x09                             - Length
 * xFF                             - Vendor specific advertising type
 * x00\x60\x52\x57\x2D\x42\x4C\x45 - "RW-BLE"
 * --------------------------------------------------------------------------------------
 */
//#define APP_SCNRSP_DATA          "\x03\x03\x83\x09\x01\x00\x00\xCA\xEA\x80"
#define APP_SCNRSP_DATA          "\x03\x03\xE7\xFE"
#define APP_SCNRSP_DATA_LENGTH  (0)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief This function handles the disconnection and also restarts the advertising or scanning
 ****************************************************************************************
 */
void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param);

/**
 ****************************************************************************************
 * @brief Initialize SPS application
 ****************************************************************************************
 */
void app_init_func(void);

/**
 ****************************************************************************************
 * @brief Send a message enable message when the connection is made
 ****************************************************************************************
 */
void app_connection_func(struct gapc_connection_req_ind const *param);

/**
 ****************************************************************************************
 * @brief Initialise the database.
 ****************************************************************************************
 */
bool app_db_init_func(void);

/**
 ****************************************************************************************
 * @brief Configure a start advertising message. Called by app_adv_start
 ****************************************************************************************
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd);

/**
 ****************************************************************************************
 * @brief Update connection parameters
 ****************************************************************************************
 */
void app_param_update_func(void);

/**
 ****************************************************************************************
 * @brief Initialize security environment.
 ****************************************************************************************
 */
void app_sec_init_func(void);

/**
 ****************************************************************************************
 * @brief Handle encryption completed event. 
 ****************************************************************************************
 */
void app_sec_encrypt_complete_func(void);

/**
 ****************************************************************************************
 * @brief 
 ****************************************************************************************
 */
void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id); 

/**
 ****************************************************************************************
 * @brief Start scanning for an advertising device.
 ****************************************************************************************
 */
void app_scanning(void); 

/**
 ****************************************************************************************
 * @brief Start the connection to the device with address connect_bdaddr
 ****************************************************************************************
 */
void app_connect(void);

/**
 ****************************************************************************************
 * @brief Stop scanning of the scanner
 ****************************************************************************************
 */
void app_cancel_scanning(void);

/**
 ****************************************************************************************
 * @brief Reset the gapm layer. This function is called when a link loss has occurred
 ****************************************************************************************
 */
void app_reset_app(void);

/**
 ****************************************************************************************
 * @brief Throughput test Function initialise application level buffers.
 ****************************************************************************************
*/
void test_pkt_init (void);

/// @} APP

#endif //APP_SPS_PROJ_H_
