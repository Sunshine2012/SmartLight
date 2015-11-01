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

#ifndef APP_SPS_UART_H_
#define APP_SPS_UART_H_

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
 #include <stdint.h>
 
 /*
 * DEFINES
 ****************************************************************************************
 */
#define FALSE	(uint8_t)0
#define TRUE	(uint8_t)1
#define ERROR	(uint8_t)2

//application defines 
#define TX_CALLBACK_SIZE 		(uint8_t)	16	//16 bytes messages
#define RX_CALLBACK_SIZE 		(uint8_t)	8

#define TX_BUFFER_ITEM_COUNT	(int)			1450 
#define TX_BUFFER_ITEM_SIZE		(uint8_t)	    1 //**do not change item_size**
#define TX_BUFFER_HWM					(int)			850   
#define TX_BUFFER_LWM					(int)			650 

#define RX_BUFFER_ITEM_COUNT	(int)			500   
#define RX_BUFFER_ITEM_SIZE		(uint8_t)	    1 //**do not change item_size**
#define RX_BUFFER_HWM					(int)			350   
#define RX_BUFFER_LWM					(int)			150   

extern uint8_t callbackbusy;
extern void uart_tx_callback(uint8_t res);
extern uint8_t tx_state_ptr;
extern uint8_t ble_data_poll_disabled;
extern uint8_t last_ble_flag;

/*
 * STRUCTURES
 ****************************************************************************************
 */
typedef struct {
	int  		item_size;
	int  		buffer_size;
	long 		byte_size;
	int	 		readIdx;
	int	 		writeIdx;
	uint8_t *data_ptr;
	int lowWaterMark;
	int	highWaterMark;
	uint8_t HWMReached;
}RingBuffer;

extern RingBuffer 	bletouart_buffer;
extern RingBuffer 	uarttoble_buffer;

///Flow control flags struct
typedef struct {
	uint8_t	txAllowed;
	uint8_t	sendXOFF;
	uint8_t sendXON;
}flags;

extern flags ble_flags;
extern flags uart_flags;
extern void checkBufferAlmostEmpty(RingBuffer* buffer, flags *flags_oppositeBuffer);

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief initialize buffers
 ****************************************************************************************
 */
void app_buffer_init(void);

/**
 ****************************************************************************************
 * @brief initialize callback functions for SPS application
 ****************************************************************************************
 */
void app_uart_init(void);

/**
 ****************************************************************************************
 * @brief reset flow control flags
 ****************************************************************************************
 */
void app_flowcontrol_init(void);

/**
 ****************************************************************************************
 * @brief push ble data to uart transmit buffer
 ****************************************************************************************
 */
void ble_push(uint8_t* wrdata, uint8_t writeAmount,uint8_t packet_type);

/**
 ****************************************************************************************
 * @brief Generate messages and provide to stream queue.
 *
 * @return void.
 ****************************************************************************************
*/
void ble_data_poll (void);

/**
 ****************************************************************************************
 * @brief 			will be used to disable buffers so that no more data can be send
 *							until they are enabled again.
 *
 * @param[in] 	flag_buffer					(which side should be disabled (uart or ble))
 * @param[in] 	state								(flow control state)
 * @return 			none
 ****************************************************************************************
 */
void updateTransmitAllowed(flags* flag_buffer, uint8_t state);

/**
 ****************************************************************************************
 * @brief Sets directly the uart flow control to xoff and sends state to opposite device
 *
 * @return void.
 ****************************************************************************************
*/
void override_ble_xoff(void);
/// @} APP

#endif //APP_SPS_UART_H_
