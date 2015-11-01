/**
****************************************************************************************
*
* @file app_sps_proj.c
*
* @brief SPS project source code.
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
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip.h"
#include "arch_sleep.h"
#include "app_task.h"                // Application task Definition
#include "app_sec.h"

#include "gapm_task.h"
#include "gapc.h"
#include "gattc_task.h"

#include "app_sps_uart.h"
#include "app_sps_proj.h"
#include "app_sps_ble.h"
#if (BLE_SPS_CLIENT)
#include "sps_client_task.h"
#endif

#include "uart_sps.h" 
#include "app_stream_queue.h"

#include "app_api.h"

#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif //(NVDS_SUPPORT)


#include <co_math.h>


extern uint8_t bat_lvl_alert_used; // aiwesky 20151005

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
*/
void app_task_custom_init(void);
void app_set_adv_data(void);
 /**
 ****************************************************************************************
 * @brief 			Initialize SPS application
 *
 * @return 			none
 ****************************************************************************************
 */
void app_init_func(void)
{	
    app_sps_init();    
    app_dis_init();          // 设备信息服务初始化  aiwesky 20151005
    app_batt_init();         // 电池服务初始化      aiwesky 20151005
    app_set_adv_data();      // 广播数据初始化      aiwesky 20151003  
    app_wechat_init();       // 微信服务初始化      aiwesky 20151007
    app_disable_sleep();
    last_ble_flag =1;
    timer0_start();
}

/**
 ****************************************************************************************
 * @brief Sends a exchange MTU command
 *
 * @return void
 ****************************************************************************************
 */

void gattc_exc_mtu_cmd(void)
{
  struct gattc_exc_mtu_cmd *cmd =  KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
            KE_BUILD_ID(TASK_GATTC,app_env.conidx ), TASK_APP,
            gattc_exc_mtu_cmd);
    
	cmd->req_type = GATTC_MTU_EXCH;

	ke_msg_send(cmd);
}

/**
 ****************************************************************************************
 * @brief Send a message enable message when the connection is made
 * 
 * @param[in] param      Connection parameters
 *
 * @return void
 ****************************************************************************************
 */
void app_connection_func(struct gapc_connection_req_ind const *param)
{	

    /*--------------------------------------------------------------
    * ENABLE REQUIRED PROFILES
    *-------------------------------------------------------------*/
    //enable SPS device role or SPS host and set the proper values
    app_sps_enable();	
	
    // aiwesky 20151004 DISS_SERVER	
    #if (BLE_DIS_SERVER)
        app_dis_enable_prf(app_env.conhdl);
    #endif
    
    // aiwesky 20101007 电池服务
    #if (BLE_BATT_SERVER)
        app_batt_enable(cur_batt_level,bat_lvl_alert_used,GPIO_PORT_1,GPIO_PIN_0);
    #endif
    
    // aiwesky 20151007 微信服务
    #if (BLE_WECHAT_SERVER)
        app_wechat_enable_prf(app_env.conhdl);
    #endif 
                

    gattc_exc_mtu_cmd();  
    
    // Setup stream API. Provide connection interval to calculate tx rate
    stream_setup_int_tomax(param->con_interval);
	
		//these functions are used by the re-ordering attributes mechanism
#if BLE_SPS_CLIENT
//		cb_reset_tx();
#endif
#if BLE_SPS_SERVER		
//		cb_reset_rx();
#endif	
		ble_data_poll_disabled=FALSE;
}

uint8_t              app_adv_data_length __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Advertising data
uint8_t              app_adv_data[ADV_DATA_LEN-3] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Scan response data length- maximum 31 bytes
uint8_t              app_scanrsp_data_length __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
/// Scan response data
uint8_t              app_scanrsp_data[SCAN_RSP_DATA_LEN] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

void app_set_adv_data(void)
{
    //  Device Name Length
    uint8_t device_name_length;

    app_adv_data_length     = APP_ADV_DATA_MAX_SIZE;
    app_scanrsp_data_length = APP_SCAN_RESP_DATA_MAX_SIZE;

    /*-----------------------------------------------------------------------------
     * Set the Advertising Data
     *-----------------------------------------------------------------------------*/
			app_adv_data_length = APP_DFLT_ADV_DATA_LEN;
			memcpy(&app_adv_data[0], APP_DFLT_ADV_DATA, app_adv_data_length);
		
    /*-----------------------------------------------------------------------------
     * Set the Scan Response Data
     *-----------------------------------------------------------------------------*/
			app_scanrsp_data_length = APP_SCNRSP_DATA_LENGTH;
			if (app_scanrsp_data_length > 0)
			{					
				memcpy(&app_scanrsp_data, APP_SCNRSP_DATA, app_scanrsp_data_length);
			}

    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_adv_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get the default Device Name to add in the Advertising Data
				device_name_length = strlen(APP_DFLT_DEVICE_NAME);
				memcpy(&app_adv_data[app_adv_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);

        // Length
        app_adv_data[app_adv_data_length]     = device_name_length + 1;
        // Device Name Flag
        app_adv_data[app_adv_data_length + 1] = '\x09';

        // Update Advertising Data Length
        app_adv_data_length = app_adv_data_length + 2;
        app_adv_data_length = app_adv_data_length + device_name_length;
    }	

    /*-----------------------------------------------------------------------------
     * Add the Device Name in the Advertising Scan Response Data
     *-----------------------------------------------------------------------------*/
    // Get available space in the Advertising Data
    device_name_length = APP_ADV_DATA_MAX_SIZE - app_scanrsp_data_length - 2;

    // Check if data can be added to the Advertising data
    if (device_name_length > 0)
    {
        // Get the default Device Name to add in the Advertising Data
				device_name_length = strlen(APP_DFLT_DEVICE_NAME);
				memcpy(&app_scanrsp_data[app_scanrsp_data_length + 2], APP_DFLT_DEVICE_NAME, device_name_length);

        // Length
        app_scanrsp_data[app_scanrsp_data_length]     = device_name_length + 1;
        // Device Name Flag
        app_scanrsp_data[app_scanrsp_data_length + 1] = '\x09';

        // Update Advertising Data Length
        app_scanrsp_data_length += (device_name_length + 2);
    }
}

/**
 ****************************************************************************************
 * @brief Configure a start advertising message. Called by app_adv_start
 * 
 * @param[in] cmd     Pointer to message.
 *
 * @return void
 ****************************************************************************************
 */
void app_adv_func(struct gapm_start_advertise_cmd *cmd)
{
    //  Device Name Length
    uint8_t device_name_length;
    uint8_t device_name_avail_space;
    uint8_t device_name_temp_buf[64];
    uint8_t Manufacturer_Specific_Data = 8;    // 厂商数据 aiwesky 20151003
   
    cmd->op.code     = GAPM_ADV_UNDIRECT;
    cmd->op.addr_src = GAPM_PUBLIC_ADDR;
    cmd->intv_min    = APP_ADV_INT_MIN;
    cmd->intv_max    = APP_ADV_INT_MAX;
    cmd->channel_map = APP_ADV_CHMAP;

    cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
    //cmd->info.host.mode = GAP_NON_DISCOVERABLE;

    /*-----------------------------------------------------------------------------------
     * Set the Advertising Data and the Scan Response Data
     *---------------------------------------------------------------------------------*/
    cmd->info.host.adv_data_len       = APP_ADV_DATA_MAX_SIZE;
    cmd->info.host.scan_rsp_data_len  = APP_SCAN_RESP_DATA_MAX_SIZE;

    // Advertising Data
#if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                &cmd->info.host.adv_data[0]) != NVDS_OK)
#endif //(NVDS_SUPPORT)
    {
        cmd->info.host.adv_data_len = APP_DFLT_ADV_DATA_LEN;
        memcpy(&cmd->info.host.adv_data[0], APP_DFLT_ADV_DATA, cmd->info.host.adv_data_len);
    }

    // Scan Response Data
#if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
#endif //(NVDS_SUPPORT)
    {
        cmd->info.host.scan_rsp_data_len = APP_SCNRSP_DATA_LENGTH;
        memcpy(&cmd->info.host.scan_rsp_data[0], APP_SCNRSP_DATA, cmd->info.host.scan_rsp_data_len);
    }

    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    device_name_avail_space = APP_ADV_DATA_MAX_SIZE - cmd->info.host.adv_data_len - 2;

    // Check if data can be added to the Advertising data
    if (device_name_avail_space > 0)
    {
        // Get the Device Name to add in the Advertising Data (Default one or NVDS one)
        #if (NVDS_SUPPORT)
        device_name_length = NVDS_LEN_DEVICE_NAME;
        if (nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, &device_name_temp_buf[0]) != NVDS_OK)
        #endif //(NVDS_SUPPORT)
        {
            // Get default Device Name (No name if not enough space)
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            memcpy(&device_name_temp_buf[0], APP_DFLT_DEVICE_NAME, device_name_length);
        }

 

        if(device_name_length > 0)
        {
            // Check available space
            device_name_length = co_min(device_name_length, device_name_avail_space);

            // Fill Length
            cmd->info.host.adv_data[cmd->info.host.adv_data_len]     = device_name_length + 1;
            // Fill Device Name Flag
            cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x09';
            // Copy device name
            memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2], device_name_temp_buf, device_name_length);
            // Update Advertising Data Length
            cmd->info.host.adv_data_len += (device_name_length + 2);
        }

        //cmd->info.host.adv_data[cmd->info.host.adv_data_len] = 2;
        //cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\x0A';
        //memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2], "\x40", 1);
        //cmd->info.host.adv_data_len += (1 + 2);

        // 广播MAC地址,微信开发需要
        cmd->info.host.adv_data[cmd->info.host.adv_data_len] = Manufacturer_Specific_Data + 1;
        cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] = '\xff';
        memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2], "\xE7\xFE\x80\xEA\xCA\x00\x00\x01", Manufacturer_Specific_Data);
        cmd->info.host.adv_data_len += (Manufacturer_Specific_Data  + 2);

        
        
    }
 
}

/**
 ****************************************************************************************
 * @brief Send a message to terminate the connection
 *
 * @param[in] dst 				Destanition 
 * @param[in] conhdl 			Connection handle
 * @param[in] reason 			Reason of termination
 *
 * @return void
 ****************************************************************************************
 */
void app_send_disconnect(uint16_t dst, uint16_t conhdl, uint8_t reason)
{
    struct gapc_disconnect_ind * disconnect_ind = KE_MSG_ALLOC(GAPC_DISCONNECT_IND,
            dst, TASK_APP, gapc_disconnect_ind);

    // fill parameters
    disconnect_ind->conhdl   = conhdl;
    disconnect_ind->reason   = reason;

    // send indication
    ke_msg_send(disconnect_ind);
}

/**
 ****************************************************************************************
 * @brief This function handles the disconnection and also restarts the advertising or scanning
 *
 * @param[in] task_id 				Destination task id.
 * @param[in] param 					Pointer to the parameters of the message. 
 *
 * @return void
 ****************************************************************************************
 */
void app_disconnect_func(ke_task_id_t task_id, struct gapc_disconnect_ind const *param)
{
	
		uint8_t state = ke_state_get(task_id);
		
		ble_data_poll_disabled=TRUE;
    
#if	(defined(CFG_APP_CENTRAL))
		if ((param->reason != CO_ERROR_REMOTE_USER_TERM_CON) &&
				(param->reason != CO_ERROR_CON_TERM_BY_LOCAL_HOST))
		{
				app_reset_app();     	// link loss (timer timeout)  //reset higher layers

				// Reset the environment **this is important because otherwise the profile att database will not be re-initiated**
				memset(&app_env, 0, sizeof(app_env));

				// Initialize next_prf_init value for first service to add in the database
				app_env.next_prf_init = APP_PRF_LIST_START + 1;
		}
		else
		{
				app_scanning();				// both devices are disconnected correctly
		}
#endif
		
#if	(defined(CFG_APP_PERIPHERAL))
		if ((state == APP_SECURITY) || (state == APP_CONNECTED)  || (state == APP_PARAM_UPD))
		{	
			// Restart Advertising
				 app_adv_start();
		}
		else
		{
				// We are not in a Connected State
				ASSERT_ERR(0);
		} 
#endif   
}

/**
 ****************************************************************************************
 * @brief Initialise the database.
 *
 * @return If database initialization completed.
 ****************************************************************************************
 */
bool app_db_init_func(void)    // 初始化所有的数据库，app_api.h每一个数据库都要调用,如APP_SPS_TASK
{  
    // Indicate if more services need to be added in the database
    bool end_db_create = false;

    // Check if another should be added in the database
    if (app_env.next_prf_init < APP_PRF_LIST_STOP)
    {
        switch (app_env.next_prf_init)
        {
            #if (BLE_SPS_SERVER)
            case (APP_SPS_TASK):
            {
                // Add SPS services to the database
                app_sps_create_db();
            } 
						break;
            #endif
						
			// aiwesky 20151004
			#if (BLE_DIS_SERVER)
			case (APP_DIS_TASK):
			{
                // Add DISS services to the database
                app_dis_create_db_send();
			} 
			break;
			#endif //BLE_DIS_SERVER
			
			// aiwesky 20151005
			#if (BLE_BATT_SERVER)
			case (APP_BASS_TASK):
			{
                app_batt_create_db();
			} 
            break;
			#endif //BLE_DIS_SERVER
					
            #if (BLE_WECHAT_SERVER)
            case (APP_WECHAT_TASK):
            {
            }
            break;
            #endif //BLE_WECHAT_SERVER
            
            default:
            {
              ASSERT_ERR(0);
            } 
						break;
        }

        // Select following service to add
        app_env.next_prf_init++;
    }
    else
    {
        end_db_create = true;
    }

    return end_db_create;
}

/**
 ****************************************************************************************
 * @brief Set the scanner role or advertiser role. Depends on BLE_SPS_CLIENT
 *
 * @param[in] task_id     ke_task_id_t kernel ID
 * @param[in] cmd     		struct gapm_set_dev_config_cmd 
 *
 * @return void
 ****************************************************************************************
 */
void app_configuration_func(ke_task_id_t const task_id, struct gapm_set_dev_config_cmd *cmd)
{
    #if (defined (CFG_APP_CENTRAL))
    {
        // Operation select
        cmd->operation = GAPM_SET_DEV_CONFIG;
        
        // Device Role
        cmd->role = GAP_CENTRAL_MST;
    }
    #elif (defined(CFG_APP_PERIPHERAL))
    {
        // set device configuration
        cmd->operation = GAPM_SET_DEV_CONFIG;
        // Device Role
        cmd->role = GAP_PERIPHERAL_SLV;
			  cmd->name_write_perm = GAPM_SET_DEV_NAME; //  aiwesky 20151003
        // Device IRK
        // cmd->irk = ; TODO NOT set

        // Device Appearance
        #if (BLE_APP_HT)
            cmd->appearance = 728;
        #else
        // Device Appearance
            cmd->appearance = 0x0000;
        #endif
        // Device Appearance write permission requirements for peer device
        cmd->appearance_write_perm = GAPM_WRITE_DISABLE;
        // Device Name write permission requirements for peer device
        cmd->name_write_perm = GAPM_WRITE_DISABLE;

        // Peripheral only: *****************************************************************
        // Slave preferred Minimum of connection interval
        cmd->con_intv_min = 8;         // 10ms (8*1.25ms)
        // Slave preferred Maximum of connection interval
        cmd->con_intv_max = 16;        // 20ms (16*1.25ms)
        // Slave preferred Connection latency
        cmd->con_latency  = 0;
        // Slave preferred Link supervision timeout
        cmd->superv_to    = 100;

        // Privacy settings bit field
        cmd->flags = 0;

        //Defined maximum transmission unit
        cmd->max_mtu = 160;
    }	
#endif			
}

/**
 ****************************************************************************************
 * @brief Called upon device's configuration completion. Starts advertsing or scanning.
 *
 * @return void.
 ****************************************************************************************
*/
void app_set_dev_config_complete_func(void)// aiwesky 20151004
{
    // We are now in Initialization State
    ke_state_set(TASK_APP, APP_DB_INIT);

    // Add the first required service in the database
    if (app_db_init())
    {
        #if (defined(CFG_APP_PERIPHERAL))
        
        // When the data base is already made start advertising	
        app_adv_start();
        
        #elif (defined(CFG_APP_CENTRAL)) 
        
        // When the data base is already made start advertising	
        app_scanning();
        
        #endif

    }
}

/**
 ****************************************************************************************
 * @brief Called upon connection param's update rejection
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/
void app_update_params_rejected_func(uint8_t status)
{
	ASSERT_INFO(0, status, APP_PARAM_UPD);
}

/**
 ****************************************************************************************
 * @brief Called upon connection param's update completion
 *
 * @return void.
 ****************************************************************************************
*/
void app_update_params_complete_func(void)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Handles undirect advertising completion.
 *
 * @return void.
 ****************************************************************************************
*/
void app_adv_undirect_complete(uint8_t status)
{
	return;
}

/**
 ****************************************************************************************
 * @brief Handles direct advertising completion.
 *
 * @param[in] status        Error code
 *
 * @return void.
 ****************************************************************************************
*/
void app_adv_direct_complete(uint8_t status)
{
    if (ke_state_get(status) == APP_CONNECTABLE)
    {
        app_adv_start(); //restart advertising
    }
}

/**
 ****************************************************************************************
 * @brief Handles Database creation. Start application.
 *
 * @return void.
 ****************************************************************************************
*/
void app_db_init_complete_func(void)
{
    // start scanning
    #if (defined(CFG_APP_CENTRAL))  
        app_scanning();
    // Start advertising
    #elif (defined(CFG_APP_PERIPHERAL)) 
        app_adv_start();
    #endif

}

/**
 ****************************************************************************************
 * @brief Start scanning for an advertising device.
 *
 * @return void
 ****************************************************************************************
 */
void app_scanning(void)
{
    ke_state_set(TASK_APP, APP_CONNECTABLE);

    // create a kernel message to start the scanning
    struct gapm_start_scan_cmd *msg = (struct gapm_start_scan_cmd *)KE_MSG_ALLOC(GAPM_START_SCAN_CMD, TASK_GAPM, TASK_APP, gapm_start_scan_cmd);
    // Maximal peer connection
    msg->mode = GAP_GEN_DISCOVERY;
    msg->op.code = GAPM_SCAN_PASSIVE;
    msg->op.addr_src = GAPM_PUBLIC_ADDR;
    msg->filter_duplic = SCAN_FILT_DUPLIC_EN;
    msg->interval =10 ;//0x180;
    msg->window = 5;

    // Send the message
    ke_msg_send(msg);   
}

/**
 ****************************************************************************************
 * @brief Start the connection to the device with address connect_bdaddr
 * 
 * @return void
 ****************************************************************************************
 */
void app_connect(void)
{
    struct gapm_start_connection_cmd *msg;
    // create a kenrel message to start connecting  with an advertiser
    msg = (struct gapm_start_connection_cmd *) KE_MSG_ALLOC(GAPM_START_CONNECTION_CMD , TASK_GAPM, TASK_APP, gapm_start_connection_cmd);
    // Maximal peer connection
    msg->nb_peers = 1;
    // Connect to a certain adress
    memcpy(&msg->peers[0].addr, &connect_bdaddr, BD_ADDR_LEN);

    msg->con_intv_min = 9; 	// number * 1.25 = connection interval 8.75 ms  = 7
    msg->con_intv_max = 9;	// number * 1.25 = connection interval 8.75 ms = 7
    msg->ce_len_min = 0x20; 
    msg->ce_len_max = 0x20;
    msg->con_latency = 0;
    msg->op.addr_src = GAPM_PUBLIC_ADDR;
    msg->peers[0].addr_type = GAPM_PUBLIC_ADDR;
    msg->superv_to = 100;   //1sec       //0x7D0;	//	20 seconden
    msg->scan_interval = 0x180;
    msg->scan_window = 0x160;
    msg->op.code = GAPM_CONNECTION_DIRECT;
	
		//set timer
    app_timer_set(APP_CONN_TIMER, TASK_APP, 700);	//7 sec

    // Send the message
    ke_msg_send(msg);    
}
/**
 ****************************************************************************************
 * @brief Handles connection timer expiration. Connection request timedout. Cancel connection procedure.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance.
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

int app_conn_timer_handler(ke_msg_id_t const msgid,
									void *param,
									ke_task_id_t const dest_id,
									ke_task_id_t const src_id)
{
    
    struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                               TASK_GAPM, TASK_APP,
                                               gapm_cancel_cmd);

    cmd->operation = GAPM_CANCEL;

    // Send the message
    ke_msg_send(cmd);
            
    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles connection request failure.
 *
 * @return void.
 ****************************************************************************************
*/

void app_connect_failed_func(void)
{
    app_scanning();
}
/**
 ****************************************************************************************
 * @brief Stop scanning of the scanner
 * 
 * @return void
 ****************************************************************************************
 */
void app_cancel_scanning(void)
{
		struct gapm_cancel_cmd *cmd =(struct gapm_cancel_cmd *) KE_MSG_ALLOC(GAPM_CANCEL_CMD, TASK_GAPM, TASK_APP,gapm_cancel_cmd);
		cmd->operation = GAPM_SCAN_PASSIVE; // Set GAPM_SCAN_PASSIVE
		ke_msg_send(cmd);// Send the message
}

/**
 ****************************************************************************************
 * @brief Reset the gapm layer. This function is called when a link loss has occurred
 * 
 * @return void
 ****************************************************************************************
 */
void app_reset_app(void)
{
		struct gapm_reset_cmd* cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP,gapm_reset_cmd);		
		cmd->operation = GAPM_RESET;// Set GAPM_RESET
		ke_msg_send(cmd);// Send the message
}

/**
 ****************************************************************************************
 * @brief Update connection parameters
 * 
 * @return void
 ****************************************************************************************
 */
void app_param_update_func(void)
{
    struct gapc_param_update_cmd * req = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD, TASK_GAPC, TASK_APP, gapc_param_update_cmd);

    // Fill in the parameter structure
    req->operation = GAPC_UPDATE_PARAMS;
    req->params.intv_min = 0xa;	    // N * 1.25ms
    req->params.intv_max = 0xa;	    // N * 1.25ms
    req->params.latency  = 0;		    // Conn Events skipped
    req->params.time_out = 1000;		    // N * 10ms
    
    ke_msg_send(req);
    
    return;
}

/* encryption functions **not used** */

/**
 ****************************************************************************************
 * @brief Initialize security environment.
 * 
 * @return void
 ****************************************************************************************
 */
void app_sec_init_func(void)
{   
		return;
}

/**
 ****************************************************************************************
 * @brief Handle encryption completed event. 
 * 
 * @return void
 ****************************************************************************************
 */
void app_sec_encrypt_complete_func(void)
{
		return;
}

/**
 ****************************************************************************************
 * @brief  
 * 
 * @return void
 ****************************************************************************************
 */
void app_mitm_passcode_entry_func(ke_task_id_t const src_id, ke_task_id_t const dest_id)
{
		return;
}

#if (BLE_APP_SEC) // the functions of BLE_APP_SEC are not used in the SPS project

/**
 ****************************************************************************************
 * @brief Handle pairring request message. Send a pairing response 
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_pairing_rsp_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    cfm->request = GAPC_PAIRING_RSP;
    cfm->accept = true;

    // OOB information
    cfm->data.pairing_feat.oob            = GAP_OOB_AUTH_DATA_NOT_PRESENT;
    // Encryption key size
    cfm->data.pairing_feat.key_size       = KEY_LEN;
    // IO capabilities
    cfm->data.pairing_feat.iocap          = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    // Authentication requirements
    cfm->data.pairing_feat.auth           = GAP_AUTH_REQ_NO_MITM_BOND;
    //Security requirements
    cfm->data.pairing_feat.sec_req        = GAP_NO_SEC;
    //Initiator key distribution
    //GZ cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_NONE;
    cfm->data.pairing_feat.ikey_dist      = GAP_KDIST_SIGNKEY;
    //Responder key distribution
    cfm->data.pairing_feat.rkey_dist      = GAP_KDIST_ENCKEY;
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Send Temporary key 
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_tk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);
    uint32_t pin_code = app_sec_gen_tk();
    cfm->request = GAPC_TK_EXCH;
    cfm->accept = true;
    
    memset(cfm->data.tk.key, 0, KEY_LEN);
    
    cfm->data.tk.key[3] = (uint8_t)((pin_code & 0xFF000000) >> 24);
    cfm->data.tk.key[2] = (uint8_t)((pin_code & 0x00FF0000) >> 16);
    cfm->data.tk.key[1] = (uint8_t)((pin_code & 0x0000FF00) >>  8);
    cfm->data.tk.key[0] = (uint8_t)((pin_code & 0x000000FF) >>  0);
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Send IRK
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_irk_exch_func(struct gapc_bond_req_ind *param)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Send CSRK
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_csrk_exch_func(struct gapc_bond_req_ind *param)
{
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_CSRK_EXCH;

    cfm->accept = true;

    memset((void *) cfm->data.csrk.key, 0, KEY_LEN);
    memcpy((void *) cfm->data.csrk.key, (void *)"\xAB\xAB\x45\x55\x23\x01", 6);

    ke_msg_send(cfm);

}

/**
 ****************************************************************************************
 * @brief Send Long term key
 * 
 * @return void
 ****************************************************************************************
 */
void app_send_ltk_exch_func(struct gapc_bond_req_ind *param)
{
    
    struct gapc_bond_cfm* cfm = KE_MSG_ALLOC(GAPC_BOND_CFM, TASK_GAPC, TASK_APP, gapc_bond_cfm);

    // generate ltk
    app_sec_gen_ltk(param->data.key_size);

    cfm->request = GAPC_LTK_EXCH;

    cfm->accept = true;

    cfm->data.ltk.key_size = app_sec_env.key_size;
    cfm->data.ltk.ediv = app_sec_env.ediv;

    memcpy(&(cfm->data.ltk.randnb), &(app_sec_env.rand_nb) , RAND_NB_LEN);
    memcpy(&(cfm->data.ltk.ltk), &(app_sec_env.ltk) , KEY_LEN);

    ke_msg_send(cfm);

}

/**
 ****************************************************************************************
 * @brief Handle Pairing/Bonding copletion event
 * 
 * @return void
 ****************************************************************************************
 */
void app_paired_func(void)
{
    return;
}

/**
 ****************************************************************************************
 * @brief Handle Pairing/Bonding copletion event
 * 
 * @return bool
 ****************************************************************************************
 */
bool app_validate_encrypt_req_func(struct gapc_encrypt_req_ind *param)
{
    return true;
}

void app_sec_encrypt_ind_func(void)
{
    
    return; 
}

#endif //BLE_APP_SEC



#endif  //BLE_APP_PRESENT
/// @} APP
