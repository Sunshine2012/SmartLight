/**
****************************************************************************************
*
* @file app_sps_uart.c
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdlib.h>
#include "uart_sps.h"
#include "app_sps_uart.h"
#include "app_sps_ble.h"
#include "gpio.h"
#include "ke_mem.h"
#include "app_stream_queue.h" 
#include "app_sps_proj.h"
// aiwesky 20150530
#include "pwm.h"

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

#if BLE_SPS_CLIENT
#define SEND_L2C_CODE L2C_CODE_ATT_WR_CMD 
#elif defined (BLE_SPS_SERVER)
#define SEND_L2C_CODE L2C_CODE_ATT_HDL_VAL_NTF 
#endif 
/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/
/// UART callback pointers
uint8_t   tx_write_pointer[TX_CALLBACK_SIZE+1] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t 	rx_read_pointer[RX_CALLBACK_SIZE] __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t 	tx_state_ptr __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t		rx_state_ptr __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY 

/// Flag to prevent multiple calls to the tx callback function
uint8_t 	callbackbusy __attribute__((section("retention_mem_area0"),zero_init));  //@RETENTION MEMORY
uint8_t		blePushCallbackIssued __attribute__((section("retention_mem_area0"),zero_init));//@RETENTION MEMORY
uint8_t 	ble_data_poll_disabled __attribute__((section("retention_mem_area0"),zero_init));  //@RETENTION MEMORY

/// SPS application buffer pointers
RingBuffer  bletouart_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
RingBuffer  uarttoble_buffer __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/// SPS application flag pointers
flags ble_flags  __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
flags uart_flags __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
uint8_t last_ble_flag __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

///Flag for enabling readItem function
uint8_t tx_buffers_full __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY
volatile int avail_tx_buffers = 0;

extern uint16_t ble_tx_hndl;

 /*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/// Private buffer functions prototypes
static void buffer_create(RingBuffer* buffer, int buffer_size, int item_size, int lowWaterMark, int highWaterMark);
static uint8_t isInitialized(RingBuffer *buffer);
static int itemCount(RingBuffer* buffer);
static void writeItems(RingBuffer* buffer, uint8_t* wrdata, uint8_t writeAmount);
static uint8_t readItems(RingBuffer* buffer, uint8_t* rddata, uint8_t readAmount);

/// Private callback functions prototypes
static void uart_rx_callback(uint8_t res);
void uart_tx_callback(uint8_t res);

/// Private application functions prototypes
static uint8_t uart_pull(uint8_t* rddata, uint8_t readamount, uint8_t* state);
static void uart_push(uint8_t* wrdata, uint8_t writeAmount, uint8_t state);

/// Private flow control functions prototypes
void checkBufferAlmostEmpty(RingBuffer* buffer, flags* flags_oppositeBuffer);
static void checkBufferAlmostFull(RingBuffer* buffer, flags* buffer_flags);
static void getFlowControlState(flags* flag_buffer , uint8_t *state);

#define MAX_TX_BUFS (18)

/**
 ****************************************************************************************
 * @brief 			function used to declare a new buffer
 *
 * @param[in] 	buffer_size		(how many items should the buffer store?)
 * @param[in] 	item_size			(how big should each item be)
 * @param[in] 	highWaterMark	(amount of items in buffer where XOFF should be send)
 * @param[in] 	lowWaterMark	(amount of items in buffer where XON should be send)
 *
 * @return 			RingBuffer pointer
 ****************************************************************************************
 */
static void buffer_create(RingBuffer* buffer, int buffer_size, int item_size, int lowWaterMark, int highWaterMark)
{
  //initialise circular buffer
  
  buffer->item_size   = item_size;
  buffer->buffer_size = buffer_size;
  buffer->byte_size   = (long)(buffer_size+1)*(long)item_size;//+1 for empty
  buffer->readIdx 	  = 0;
  buffer->writeIdx 	  = 0;
	
	//memory allocation of circular buffer
	buffer->data_ptr 	= ke_malloc((long)(buffer->byte_size)*sizeof(uint8_t), KE_MEM_NON_RETENTION); 
	
	//flow control
	buffer->HWMReached	= FALSE;
	
	//check to make sure that watermarks are valid
	if(highWaterMark < buffer_size && 
		lowWaterMark <= highWaterMark && 
		lowWaterMark > 0)
	{
		buffer->highWaterMark = highWaterMark;
		buffer->lowWaterMark  = lowWaterMark;
	}
	else
	{
			while(1); //error: watermarks invalid
	}
	
	return;
}

/**
 ****************************************************************************************
 * @brief 			function used to check if buffer is initialized
 *
 * @param[in] 	buffer		(buffer to check)
 *
 * @return 			TRUE(1) or FALSE(0)
 ****************************************************************************************
 */
static uint8_t isInitialized(RingBuffer *buffer)
{
		if(buffer != NULL &&
			 buffer->data_ptr != NULL)
		{
				return TRUE;			
		}
		return FALSE;
}

/**
 ****************************************************************************************
 * @brief 			function used to count the amount of items in buffer
 *
 * @param[in] 	buffer		(buffer to check)
 *
 * @return 			amount of items
 ****************************************************************************************
 */
static int itemCount(RingBuffer* buffer)
{
		int wr = buffer->writeIdx;
		int rd = buffer->readIdx;
	
		//count items in buffer
		if(wr >= rd)
		{
				return (wr - rd);
		}
		else
		{
				return ((wr + buffer->byte_size) - rd);
		}
}

/**
****************************************************************************************
* @brief 		 	function used to push multiple items to buffer **buffer will not check if its full yet**
*
* @param[in] 	buffer                  (buffer to push data)
* @param[in] 	wrdata                  (pointer with data which should be pushed)
* @param[in] 	writeAmount						 (amount of bytes which should be pushed to buffer)
* @param[in] 	flowcontrol_ptr         (not used yet)
*
* @return 		none
****************************************************************************************
*/
static void writeItems(RingBuffer* buffer, uint8_t* wrdata, uint8_t writeAmount)
{		
    //before writing items check how much room there is available and adjust write amount if there is almost no room
    int items = itemCount(buffer);
    signed int roomLeft = (buffer->buffer_size - items);

    if(roomLeft > 0 )
    {
        if(roomLeft < writeAmount)
        {
            //buffer does not have enough room to store all data.
            writeAmount = roomLeft;
        }	
        
        //check if writeindex has reached his limit
        signed int ptr_overflow = (((signed int)buffer->writeIdx + writeAmount) - (buffer->byte_size));
        
        //writeindex has not reached his limit, therefore its safe to write all data
        if(ptr_overflow <= 0)
        {
            //write all data
            memcpy((buffer->data_ptr)+buffer->writeIdx, wrdata, writeAmount);

            //update write index
            buffer->writeIdx += writeAmount;																									
        }
        else
        {
            //write until limit is reached and write remaining data at wr_idx == 0
            memcpy((buffer->data_ptr)+buffer->writeIdx, wrdata, (writeAmount-ptr_overflow));		
            memcpy((buffer->data_ptr), wrdata+(writeAmount-ptr_overflow), ptr_overflow);
            
            //update write index
            buffer->writeIdx = ptr_overflow;																									
        }	
    }

}

/**
****************************************************************************************
* @brief 			function used to pull multiple items from buffer
*
* @param[in] 	buffer                  (buffer to pull from)
* @param[in] 	rddata                  (pulled data)
* @param[in] 	maxReadAmount           (maximum items to pull)
* @param[in] 	flowcontrol_ptr         (not used yet)
*
* @return 		none
****************************************************************************************
*/
static uint8_t readItems(RingBuffer* buffer, uint8_t* rddata, uint8_t readAmount)
{
    //check if there are items to read in the buffer and adjust read amount if the buffer is almost empty
    signed int itemsInBuffer = itemCount(buffer);	
    if(itemsInBuffer > 0)
    {
        if(itemsInBuffer < readAmount){
            readAmount = itemsInBuffer;
        }	
            
        //check if read index has reached the limit
        signed int ptr_overflow = (((signed int)buffer->readIdx + readAmount) - (buffer->byte_size));
        
        //if not, just read everything at once
        if(ptr_overflow <= 0){
            //pull data and update read index
            memcpy(rddata, (buffer->data_ptr+buffer->readIdx), readAmount);
            buffer->readIdx += readAmount;
                    
            //terminate for safety
            rddata[readAmount] = 0;  
        }
        else
        {
            //pull data until maximum reached and pull remaining data from rd_idx == 0
            memcpy(rddata, (buffer->data_ptr+buffer->readIdx), (readAmount-ptr_overflow));
            memcpy(rddata+(readAmount-ptr_overflow), buffer->data_ptr, ptr_overflow);
                    
            //update index
            buffer->readIdx = ptr_overflow;
                    
            //terminate for safety
            rddata[readAmount] = 0;
        }
    }
    //if there are no items in buffer
    else 
    {
        //reset readamount so that application will know that no bytes are pulled
        readAmount = 0;

        //terminate for safety
        rddata[0] = 0; 		
    }
    
    return readAmount; //return the amount of items pulled from buffer
}

/**
 ****************************************************************************************
 * @brief 			receive callback function will handle incoming uart data
 *
 * @param[in] 	res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_TIMEOUT)
 *
 * @return 			none
 ****************************************************************************************
 */

static void uart_rx_callback(uint8_t res)
{	
    static unsigned int count = 0;
    //function called from uart receive isr
    if(res == UART_STATUS_OK) 
    {	
        uart_push(rx_read_pointer, RX_CALLBACK_SIZE, rx_state_ptr);
    }

    //function called from uart timeout isr	
    else if(res == UART_STATUS_TIMEOUT) 
    {
        int size = 0;
        while((rx_read_pointer[size]) != 0)
        {
            size++;
        }
            
        uart_push(rx_read_pointer, size, rx_state_ptr);
        count = (count + 200) % 16000;
			
		if(rx_read_pointer[0] == 'a')
		{
		    timer0_set_pwm_high_counter(count);
            timer0_set_pwm_low_counter(16000-count);
	        //SetWord16(P1_DATA_REG,~(GetWord16(P1_DATA_REG)) | 0xfe);	
		}
        // PC 到手机端的数据
        // uart_push("aiwesky\n", 8, rx_state_ptr);
    }
    else
    {
        while(1); //error: callback called from unknown source
    }
    
    //reinitiate callback
    uart_sps_read(rx_read_pointer, RX_CALLBACK_SIZE, &rx_state_ptr, &uart_rx_callback); 
}

/**
 ****************************************************************************************
 * @brief 			transmit callback function will handle uart data transmission
 *
 * @param[in] 	res (status: UART_STATUS_OK, UART_STATUS_ERROR, UART_STATUS_TIMEOUT)
 *
 * @return 			none
 ****************************************************************************************
 */
void uart_tx_callback(uint8_t res)
{	
	
    //function gets called from uart transmit isr or application when its not running
    if(res == UART_STATUS_OK) 
    {
        //reset state pointer
        tx_state_ptr = UART_NONE;			

        //get data and pointer
        uint8_t size = uart_pull(tx_write_pointer, TX_CALLBACK_SIZE, &tx_state_ptr);	

        //if there is data available, send data over uart
        if(size > 0)
        {
        // 手机端到电脑
            uart_sps_write(tx_write_pointer, size, &tx_state_ptr, &uart_tx_callback);
			  //uart_sps_write("haha", 4, &tx_state_ptr, &uart_tx_callback);
            return;
        }	

        //if there is no data but only flow control just send flow control to UART
        else if(tx_state_ptr == UART_XOFF || tx_state_ptr == UART_XON){
            uart_sps_write(0, 0, &tx_state_ptr, &uart_tx_callback);
            return;
        }
    }
    else
    {
        while(1); //error: callback called from unknown source
    }
    
    //there is no data in the buffer so the callback is done
    callbackbusy = FALSE; 
}

/**
 ****************************************************************************************
 * @brief 			This function checks if the buffer is almost empty. If it is disabled because
 *							it was full, it will be enabled again by issueing a XON.
 *
 * @param[in] 	buffer							(buffer to check)
 * @param[in] 	buffer_flags				(flags to issue XON)
 *
 * @return 			none
 ****************************************************************************************
 */
void checkBufferAlmostEmpty(RingBuffer* buffer, flags *buffer_flags)
{
		// if the buffer has emptied enough to continue receiving data issue a XON
		if(itemCount(buffer) <= buffer->lowWaterMark)
		{
			if(buffer->HWMReached == TRUE)
			{
				//issue XON
				buffer->HWMReached 			= FALSE;
				buffer_flags->sendXON		= TRUE;
			}	
		}	
}

/**
 ****************************************************************************************
 * @brief 			This function checks if the buffer is almost full. If so, a XOFF will be
 *							issued.
 *
 * @param[in] 	buffer							(buffer to check)
 * @param[in] 	buffer_flags				(flags to issue XOFF)
 *
 * @return 			none
 ****************************************************************************************
 */
static void checkBufferAlmostFull(RingBuffer* buffer, flags* buffer_flags)
{
		if(itemCount(buffer) > buffer->highWaterMark && buffer->HWMReached == FALSE)	//if high watermark exceeded
		{
			//issue XOFF
			buffer_flags->sendXOFF	= TRUE;				
			buffer->HWMReached 			= TRUE;
		}		
}

/**
 ****************************************************************************************
 * @brief 			This function consumes the sendXON and sendXOFF flag of uart or ble and
 *							change the state according to that. the state will eventually be send
 *							over UART or BLE.
 *
 * @param[in] 	flag_buffer					(which side should be disabled (UART or BLE))
 * @param[in] 	state								(flow control state)
 *
 * @return 			none
 ****************************************************************************************
 */
static void getFlowControlState(flags* flag_buffer , uint8_t *state)
{		
		// check if the flag is not equal
		if((flag_buffer->sendXON != flag_buffer->sendXOFF) && *state == UART_NONE)
		{
			if (flag_buffer->sendXON == TRUE)
			{ 
				// set state to transmit a XON
				*state = UART_XON;
			}
			else 
			{
				// set state to transmit a XOFF
				*state = UART_XOFF;
			} 
		} 
		else
		{
			*state = UART_NONE; //reset
		}
		
		// consume flags
		flag_buffer->sendXON  = FALSE;
		flag_buffer->sendXOFF = FALSE;
}

/**
 ****************************************************************************************
 * @brief 			will be used to disable buffers so that no more data can be send
 *							until they are enabled again.
 *
 * @param[in] 	flag_buffer					(which side should be disabled (uart or ble))
 * @param[in] 	state								(flow control state)
 *
 * @return 			none
 ****************************************************************************************
 */
void updateTransmitAllowed(flags* flag_buffer, uint8_t state)
{	
		if(state == UART_XOFF) 
		{
			//disable buffer if xoff is received
			flag_buffer->txAllowed = FALSE;
		}	
		else if(state == UART_XON) 
		{
			//if xon is received, enable buffer
			flag_buffer->txAllowed = TRUE;
		}
		else 
		{
			return; //do nothing
		}
}

/**
 ****************************************************************************************
 * @brief 		pull data to transmit over UART
 *
 * @param[in] 	rddata			(pulled data, will be transmitted by callback)
 * @param[in] 	readAmount	(how much bytes should be pulled?)
 * @param[in] 	state				(flow control state from the SDK to be send over UART)
 *
 * @return 			readcount		(how much items are actually pulled?)
 ****************************************************************************************
 */
static uint8_t uart_pull(uint8_t* rddata, uint8_t readamount, uint8_t *state)
{
		uint8_t readcount = 0;
		
#if (UART_SW_FLOW_ENABLED)
		getFlowControlState(&uart_flags, state);
#endif
#if (UART_HW_FLOW_ENABLED)
		getFlowControlState(uart_flags, state);
#endif
	
		//only pull data if it's allowed
		if(uart_flags.txAllowed == TRUE)
		{
			//pull data
			readcount = readItems(&bletouart_buffer, rddata, readamount);
			//check if buffer is almost empty and send XON if neccesary
			checkBufferAlmostEmpty(&bletouart_buffer, &ble_flags);
	
			//if XON should be send, make sure it's send as soon as possible
			if(ble_flags.sendXON == TRUE){
				sendFlowControlOverBluetooth(UART_XON);
				last_ble_flag=1;
				ble_flags.sendXON = UART_NONE;
			}
		}		

		return readcount;
}

/**
 ****************************************************************************************
 * @brief 		push uart data to buffer and update flow control 
 *
 * @param[in] 	wrdata					(data)
 * @param[in] 	writeAmount			(how much bytes should be stored)
 * @param[in] 	state						(has the uart driver received a flow control state?)
 *
 * @return 			none
 ****************************************************************************************
 */
static void uart_push(uint8_t* wrdata, uint8_t writeAmount, uint8_t state)
{	
	//save old pullAllowed state and update pullAllowed depending on incoming state
	uint8_t pullAllowedOld = uart_flags.txAllowed;
	updateTransmitAllowed(&uart_flags, state); 
	
	//write items to buffer
	writeItems(&uarttoble_buffer, wrdata, writeAmount);
	
	//check if buffer is almost full and issue to send a XOFF if so
	checkBufferAlmostFull(&uarttoble_buffer, &uart_flags);

	//make sure that XOFF is send as fast as possible
	if(uart_flags.sendXOFF == TRUE || pullAllowedOld != uart_flags.txAllowed)
	{
		__disable_irq();
		if(!callbackbusy){
			callbackbusy = TRUE;
			uart_tx_callback(UART_STATUS_OK);
		}
		__enable_irq();
	}
}

/**
 ****************************************************************************************
 * @brief 			Push Bluetooth data to buffer or update flow control depending on packet
 *							type.
 * @param[in] 	wrdata					(data)
 * @param[in] 	writeAmount			(how much bytes should be stored)
 * @param[in] 	packet_type			(1 = flow control, 0 = data)
 *
 * @return 			none
 ****************************************************************************************
 */
void ble_push(uint8_t* wrdata, uint8_t writeAmount,uint8_t packet_type)
{
		
		//if a data packet is received
		if (packet_type == 0)
		{
			//make sure that only one function can call the tx callback
			__disable_irq();
			if(!callbackbusy){		
				callbackbusy = TRUE;
				blePushCallbackIssued = TRUE;
			}
			
			//write items to buffer;
			writeItems(&bletouart_buffer, wrdata, writeAmount);
			
			//check if buffer is almost full and issue to send a XOFF if so
			checkBufferAlmostFull(&bletouart_buffer, &ble_flags);
			
			//if XOFF must be send, send asap
			if(ble_flags.sendXOFF	== TRUE){
				sendFlowControlOverBluetooth(UART_XOFF);
				last_ble_flag=0;
				ble_flags.sendXOFF = UART_NONE;
			}			
			
			//only callback when it's not busy
			if(blePushCallbackIssued){
				//consume flag
				blePushCallbackIssued = FALSE;
				
				//start transmitting
				uart_tx_callback(UART_STATUS_OK);
			}	
			__enable_irq();
		}	

}

/**
 ****************************************************************************************
 * @brief 			Initialise buffers
 *
 * @return 			none
 ****************************************************************************************
 */
void app_buffer_init()
{
		if(!isInitialized(&bletouart_buffer) && !isInitialized(&uarttoble_buffer))
		{
			//initialize buffers
			buffer_create(&bletouart_buffer, TX_BUFFER_ITEM_COUNT, TX_BUFFER_ITEM_SIZE, TX_BUFFER_LWM, TX_BUFFER_HWM);
			buffer_create(&uarttoble_buffer, RX_BUFFER_ITEM_COUNT, RX_BUFFER_ITEM_SIZE, RX_BUFFER_LWM, RX_BUFFER_HWM);
		}
}

/**
 ****************************************************************************************
 * @brief 			Enable uart communication and callbacks
 *
 * @return 			none
 ****************************************************************************************
 */
void app_uart_init()
{
		//call read function once to initialize uart driver environment
		uart_sps_read(rx_read_pointer, RX_CALLBACK_SIZE, &rx_state_ptr, &uart_rx_callback);	
}

/**
 ****************************************************************************************
 * @brief 			Initialize streaming application flow control flags
 *
 * @return 			none
 ****************************************************************************************
 */
void app_flowcontrol_init()
{
		
		//set flags to default
		ble_flags.sendXOFF		= FALSE;
		ble_flags.sendXON			= FALSE;
		ble_flags.txAllowed		= TRUE;
	
		uart_flags.sendXOFF		= FALSE;
		uart_flags.sendXON		= FALSE;
		uart_flags.txAllowed	= TRUE;
}

#if (STREAMDATA_QUEUE)

/**
 ****************************************************************************************
 * @brief Callback function. Registered when packet is provided to stream queue. Called when packet is sent.
 *
 * @param[in] 	addr					Pointer to buffer
 * @param[in] 	handle				
 *
 * @return void.
 ****************************************************************************************
*/

void poll_callback (void* addr, int handle)
{
	avail_tx_buffers--;
	if (avail_tx_buffers<0){
		avail_tx_buffers=0;
	}	else if (avail_tx_buffers<(MAX_TX_BUFS)){
		tx_buffers_full = FALSE;
	}
	if (addr)
		ke_free(addr);

}

/**
 ****************************************************************************************
 * @brief Generate messages and provide to stream queue.
 *
 * @return void.
 ****************************************************************************************
*/

void ble_data_poll (void)
{

	uint8_t* rddata;
	uint8_t rddata_size;

	if (!ble_data_poll_disabled){
		if(ble_flags.txAllowed == TRUE){		
			do{
				if (tx_buffers_full == FALSE){		
					rddata = ke_malloc(21, KE_MEM_NON_RETENTION);  
					if (rddata>0){
						if ((rddata_size = readItems(&uarttoble_buffer,rddata ,20))>0){
							avail_tx_buffers++;
							if (avail_tx_buffers >= MAX_TX_BUFS){
								tx_buffers_full=TRUE;
							} 
						} else {
							ke_free(rddata);
							break;
						}
					} else
						break;
				} else
					break;
			} while (stream_fifo_add (rddata , rddata_size, ble_tx_hndl, SEND_L2C_CODE, poll_callback)>0 );  //send as many as possible
				
			//check if buffer is almost empty and send XON if neccesary
			checkBufferAlmostEmpty(&uarttoble_buffer, &uart_flags);
				
			//if XON should be send, make sure it's send as soon as possible
			if(uart_flags.sendXON == TRUE){
				__disable_irq();
				if(!callbackbusy){
					callbackbusy = TRUE;
					uart_tx_callback(UART_STATUS_OK);
				}
				__enable_irq();
			}
		}
	}
}

/**
 ****************************************************************************************
 * @brief Sets directly the uart flow control to xoff and sends state to connected device
 *
 * @return void.
 ****************************************************************************************
*/
void override_ble_xoff(void)
{
	bletouart_buffer.HWMReached = TRUE;
	uart_flags.txAllowed = FALSE;
	sendFlowControlOverBluetooth(UART_XOFF);
	last_ble_flag=0;
}
#endif //STREAMDATA_QUEUE

/// @} APP


