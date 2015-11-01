/**
 ****************************************************************************************
 *
 * @file uart_spp.c
 *
 * @brief UART driver for SPP
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
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include "uart_sps.h"       // uart definition
#include "reg_uart.h"   // uart register
#include "hcic_eif.h"   // hci uart definition
#include "global_io.h"
#include "rwble_config.h"

/*newly added*/
#include "app_sps_uart.h"

/*
 * DEFINES
 *****************************************************************************************
 */
 
/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
enum UART_ID
{
    MODEM_STAT         = 0,
    NO_INT_PEND        = 1,
    THR_EMPTY          = 2,
    RECEIVED_AVAILABLE = 4,    
    UART_TIMEOUT       = 12
};

/// RX_LVL values
enum UART_RXLVL
{
    UART_RXLVL_1,
    UART_RXLVL_4,
    UART_RXLVL_8,
    UART_RXLVL_14
};

/// WORD_LEN values
enum UART_WORDLEN
{
    UART_WORDLEN_5,
    UART_WORDLEN_6,
    UART_WORDLEN_7,
    UART_WORDLEN_8
};

/// FIFO_SZ values
enum UART_FIFOSIZE
{
    UART_FIFOSIZE_16,
    UART_FIFOSIZE_32,
    UART_FIFOSIZE_64,
    UART_FIFOSIZE_128
};

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_txrxchannel
{

   uint32_t  size;     // size
   uint8_t  *bufptr;   // buffer pointer
   uint8_t  *state;	// flow control state
   void (*callback) (uint8_t);    // call back function pointer

};

/// UART environment structure
struct uart_env_tag
{
    
    struct uart_txrxchannel tx;    // tx channel
    struct uart_txrxchannel rx;    // rx channel
    uint8_t errordetect;    // error detect
    
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// uart environment structure
static struct uart_env_tag uart_sps_env __attribute__((section("retention_mem_area0"),zero_init)); //@RETENTION MEMORY

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static uint8_t readData = 0;							//uart receive data will be read into this variable
static uint8_t stuffingByteReceived = 0;	//if flow control and byte stuffing is enabled, this variable will be used
static uint8_t stuffingByteSend = 0; 			//if flow control and byte stuffing is enabled, this variable will be used

/* test variables */
long debug_uart_nb_bytes_send = 0;
long debug_uart_nb_bytes_received = 0;
/* end */

/**
 ****************************************************************************************
 * @brief Checks if there are available data in FIFO. It is called by uart timeout
 *        interrupt.
 *
 * @return void
 ****************************************************************************************
*/
static void uart_sps_timeout_data_avail_isr(void)
{
		void (*callback) (uint8_t);
	  
		//get remaining data
		while (uart_data_rdy_getf())
		{
			 // Read the received in the FIFO
				readData = uart_rxdata_getf();

#if (UART_BYTESTUFFING_ENABLED && UART_SW_FLOW_ENABLED)
				if(stuffingByteReceived)
#endif /*bytestuffing enabled*/
				{
#if (UART_SW_FLOW_ENABLED)
						//reset flag
						stuffingByteReceived = 0;
					
						if(readData == UART_XON_BYTE)
						{
								//callback function will return that a XON is received
								*uart_sps_env.rx.state = UART_XON;
						}
						else if(readData == UART_XOFF_BYTE)
						{
								//callback function will return that a XON is received
								*uart_sps_env.rx.state = UART_XOFF;
								override_ble_xoff();
						}
						else if(readData == UART_STUFFING_BYTE)
						{
								//put data in buffer
								*uart_sps_env.rx.bufptr = readData; 
								
								//update RX parameters
								uart_sps_env.rx.size--;
								uart_sps_env.rx.bufptr++;
						}
#if (UART_BYTESTUFFING_ENABLED)							
						else
						{
								return;//this should never happen
						}	
				}
				else
				{		
						if(readData == UART_STUFFING_BYTE)
						{
								//set flag
								stuffingByteReceived = 1;
						}
#endif /*bytestuffing enabled*/
						else
#endif /*flow control enabled*/
						{
								//reset flag
								stuffingByteReceived = 0;
							
								//put data in buffer
								*uart_sps_env.rx.bufptr = readData;
								debug_uart_nb_bytes_received++;						
								
								//update RX parameters
								uart_sps_env.rx.bufptr++;
						}
				}
		}

		*uart_sps_env.rx.bufptr =  0; //terminate pointer

		// Reset RX parameters
		uart_sps_env.rx.bufptr = NULL;
		uart_sps_env.rx.size = 0;

		// Retrieve callback pointer
		callback = uart_sps_env.rx.callback;
		
		// Clear callback pointer
		uart_sps_env.rx.callback = NULL;

		// Call callback
		callback(UART_STATUS_TIMEOUT);		
}


/**
 ****************************************************************************************
 * @brief Serves the receive data available interrupt requests. It clears the requests and
 *        executes the callback function.
 *
 * @return void
 ****************************************************************************************
 */
static void uart_sps_rec_data_avail_isr(void)
{
    void (*callback) (uint8_t) = NULL;
	
    while (uart_data_rdy_getf())
    {
        // Read the received in the FIFO
				readData = uart_rxdata_getf();
			
#if (UART_BYTESTUFFING_ENABLED && UART_SW_FLOW_ENABLED)
					if(stuffingByteReceived)
#endif /*bytestuffing enabled*/
					{
#if (UART_SW_FLOW_ENABLED)
							//reset flag
							stuffingByteReceived = 0;
						
							if(readData == UART_XON_BYTE)
							{
									//callback function will tell application that XOFF is received
									*uart_sps_env.rx.state = UART_XON;
							}
							else if(readData == UART_XOFF_BYTE)
							{
									//callback function will tell application that XON is received
									*uart_sps_env.rx.state = UART_XOFF;
									override_ble_xoff();

							}
							else if(readData == UART_STUFFING_BYTE)
							{
									//put data in buffer
									*uart_sps_env.rx.bufptr = readData; 
							
									//update RX parameters
									uart_sps_env.rx.size--;
									uart_sps_env.rx.bufptr++;
							}
#if (UART_BYTESTUFFING_ENABLED)							
							else
							{
									return;//this should never happen
							}	
					}
					else
					{		
						
							if(readData == UART_STUFFING_BYTE)
							{
									//set flag
									stuffingByteReceived = 1;
							}
#endif /*bytestuffing enabled*/
							else
#endif /*flow control enabled*/
							{
									//reset flag
									stuffingByteReceived = 0;
								
									//put data in buffer
									*uart_sps_env.rx.bufptr = readData;
									debug_uart_nb_bytes_received++;

									//update RX parameters
									uart_sps_env.rx.size--;
									uart_sps_env.rx.bufptr++;
							}
					}
				
        // Check if all expected data have been received
        if (uart_sps_env.rx.size == 0)
        {
            // Reset RX parameters
            uart_sps_env.rx.bufptr = NULL;

            // Retrieve callback pointer
            callback = uart_sps_env.rx.callback;

            if(callback != NULL)
            {
                // Clear callback pointer
                uart_sps_env.rx.callback = NULL;

                // Call handler
                callback(UART_STATUS_OK);
            }
            else
            {
                ASSERT_ERR(0);
            }

            // Exit loop
            break;
        }
		}
}

/**
 ****************************************************************************************
 * @brief Serves the receive data error interrupt requests. It clears the requests and
 *        executes the callback function.
 *
 * @return void
 ****************************************************************************************
 */

static void uart_sps_rec_error_isr(void)
{
    void (*callback) (uint8_t) = NULL;
    // Reset RX parameters
    uart_sps_env.rx.size = 0;
    uart_sps_env.rx.bufptr = NULL;

    // Disable RX interrupt
    SetBits16(UART_IER_DLH_REG, ERBFI_dlh0, 0);

    // Retrieve callback pointer
    callback = uart_sps_env.rx.callback;

    if(callback != NULL)
    {
        // Clear callback pointer
        uart_sps_env.rx.callback = NULL;

        // Call handler
        callback(UART_STATUS_ERROR);
    }
    else
    {
        ASSERT_ERR(0);
    }
}

/**
 ****************************************************************************************
 * @brief Serves the transmit data fill interrupt requests. It clears the requests and
 *        executes the callback function. The callback function is called as soon as the
 *        last byte of the provided data is put into the FIFO. The interrupt is disabled
 *        at the same time.
 *
 * @return void
 ****************************************************************************************
 */
static void uart_sps_thr_empty_isr(void)
{
    void (*callback) (uint8_t) = NULL;
	
    // Fill TX FIFO until there is no more room inside it
    while (uart_txfifo_full_getf())
    {
#if (UART_BYTESTUFFING_ENABLED && UART_SW_FLOW_ENABLED)
				
				if((*uart_sps_env.tx.state == UART_XOFF ||
						*uart_sps_env.tx.state == UART_XON) &&
						stuffingByteSend == 0)
				{
						uart_txdata_setf(UART_STUFFING_BYTE); //push UART stuffing byte
						stuffingByteSend = 1;
				}
				else		
#endif /*stuffing*/	
				{											
						stuffingByteSend = 0;	//reset

#if (UART_SW_FLOW_ENABLED)
						if(*uart_sps_env.tx.state == UART_NONE)
#endif	
						{
								// Put a byte in the FIFO
								uart_txdata_setf(*uart_sps_env.tx.bufptr); 
								debug_uart_nb_bytes_send++;
								
								// Update TX parameters
								uart_sps_env.tx.size--;
								uart_sps_env.tx.bufptr++;	
						}
						
#if (UART_SW_FLOW_ENABLED)
						else if(*uart_sps_env.tx.state == UART_XOFF)
						{
								uart_txdata_setf(UART_XOFF_BYTE); //push UART XOFF byte
						}
						else if (*uart_sps_env.tx.state  == UART_XON)
						{
								uart_txdata_setf(UART_XON_BYTE); //push UART XON byte
						}
						else //undefined state
						{
								while(1); //this should never happen
						}
						*uart_sps_env.tx.state = UART_NONE; //just send data from now on				
#endif
				}
				if (uart_sps_env.tx.size == 0 && *uart_sps_env.tx.state == UART_NONE)
				{		
						// Reset TX parameters
						uart_sps_env.tx.bufptr = NULL;

						// Disable TX interrupt	
						uart_thr_empty_setf(0);
					
						// Retrieve callback pointer
						callback = uart_sps_env.tx.callback;

						if(callback != NULL)
						{
								// Clear callback pointer
								uart_sps_env.tx.callback = NULL;

								// Call handler
								callback(UART_STATUS_OK);
						}
						else
						{
								ASSERT_ERR(0);
						}

						// Exit loop
						break;
				}
		}
}

/**
 ****************************************************************************************
 * @brief Check if RX FIFO is empty.
 *
 * @return FIFO empty      false: not empty  / true: empty
 *****************************************************************************************
 */
static bool uart_sps_is_rx_fifo_empty(void)
{
    return (uart_data_rdy_getf() == 0);
}

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void uart_sps_init(uint8_t baudr, uint8_t mode )
{
    //ENABLE FIFO, REGISTER FCR IF UART_LCR_REG.DLAB=0
    SetBits16(UART_LCR_REG, UART_DLAB, 0);
			
    // XMIT FIFO RESET, RCVR FIFO RESET, FIFO ENABLED,
    SetWord16(UART_IIR_FCR_REG,0x87); //rcv int when rx fifo 1/2 full
    //SetWord16(UART_IIR_FCR_REG,0xC7); //rcv int when rx fifo 14/16 full	/* this option will cause the sps application to overflow the Rx FIFO */
	
    //DISABLE INTERRUPTS, REGISTER IER IF UART_LCR_REG.DLAB=0
    SetWord16(UART_IER_DLH_REG, 0);

    // ACCESS DIVISORLATCH REGISTER FOR BAUDRATE, REGISTER UART_DLH/DLL_REG IF UART_LCR_REG.DLAB=1
    SetBits16(UART_LCR_REG, UART_DLAB, 1);
    SetWord16(UART_IER_DLH_REG,0); 

    SetWord16(UART_IER_DLH_REG, (baudr&0xFF>>0x8));
    SetWord16(UART_RBR_THR_DLL_REG,baudr&0xFF); 

    // NO PARITY, 1 STOP BIT, 8 DATA LENGTH
    SetWord16(UART_LCR_REG, mode);

    //ENABLE TX INTERRUPTS, REGISTER IER IF UART_LCR_REG.DLAB=0
    SetBits16(UART_LCR_REG, UART_DLAB, 0);

	  NVIC_EnableIRQ(UART_IRQn);
    NVIC->ICPR[UART_IRQn]=1; //clear eventual pending bit, but not necessary becasuse this is already cleared automatically in HW

    // Configure UART environment
    uart_sps_env.errordetect = UART_ERROR_DETECT_DISABLED;
    uart_sps_env.rx.bufptr = NULL;
    uart_sps_env.rx.state	 = NULL;
    uart_sps_env.rx.size = 0;
    uart_sps_env.tx.bufptr = NULL;
	  uart_sps_env.tx.state	 = NULL;
    uart_sps_env.tx.size = 0;

	uart_sps_flow_off(); //turn rts/cts flow control off
    //SetWord32(UART_MCR_REG, UART_AFCE|UART_RTS); //Enable RTS/CTS handshake /*is not tested in sps application*/
		//uart_write("uart_is_initialised",19);
}

void uart_sps_flow_on(void)
{
    // Configure modem (HW flow control enable)
    SetWord32(UART_MCR_REG, UART_AFCE|UART_RTS);
}

bool uart_sps_flow_off(void)
{
    bool flow_off = true;

    do
    {
        // First check if no transmission is ongoing
        if ((uart_temt_getf() == 0) || (uart_thre_getf() == 0))
        {
            flow_off = false;
            break;
        }

        // Configure modem (HW flow control disable, 'RTS flow off')
        // Wait for 1 character duration to ensure host has not started a transmission at the
        // same time
        #ifndef __GNUC__
//        timer_wait(UART_CHAR_DURATION);
        #endif //__GNUC__

        // Check if data has been received during wait time
        if(!uart_sps_is_rx_fifo_empty())
        {
            // Re-enable UART flow
            uart_sps_flow_on();

            // We failed stopping the flow
            flow_off = false;
        }
    } while(false);

    return (flow_off);
}

void uart_sps_finish_transfers(void)
{
    // Configure modem (HW flow control disable, 'RTS flow off')
    uart_mcr_pack(UART_ENABLE,     // extfunc
                  UART_DISABLE,    // autorts
                  UART_ENABLE,     // autocts
                  UART_DISABLE);   // rts

    // Wait TX FIFO empty
    while(!uart_thre_getf() || !uart_temt_getf());
}

void uart_sps_read(uint8_t *bufptr, uint32_t size, uint8_t *state, void (*callback) (uint8_t))
{
    // Sanity check
    ASSERT_ERR(bufptr != NULL);
    ASSERT_ERR(size != 0);
    ASSERT_ERR(uart_sps_env.rx.bufptr == NULL);

    // Prepare RX parameters

    uart_sps_env.rx.size 		= size;
    uart_sps_env.rx.bufptr 		= bufptr;     
    uart_sps_env.rx.state 		= state;
    uart_sps_env.rx.callback 	= callback; 
	
    // Start data transaction
    uart_rec_data_avail_setf(1); //=SetBits16(UART_IER_DLH_REG, ETBEI_dlh0, 1); 
}

void uart_sps_write(uint8_t *bufptr, uint32_t size, uint8_t *state, void (*callback) (uint8_t))
{
   // Sanity check
    ASSERT_ERR(bufptr != NULL);
		if(*state == UART_NONE && !UART_SW_FLOW_ENABLED) //size could be 0 if only flow control must be send
		{
			ASSERT_ERR(size != 0);
		}
    ASSERT_ERR(uart_sps_env.tx.bufptr == NULL);
    // Prepare TX parameters
    uart_sps_env.tx.size 			= size;
    uart_sps_env.tx.bufptr 		= bufptr;
    uart_sps_env.tx.state 		= state;
	 uart_sps_env.tx.callback 	= callback; 
			
    uart_thr_empty_setf(1);
}

#ifdef __GNUC__
void __real_UART_Handler(void);
#else
void $Super$$UART_Handler(void);
#endif

#ifdef __GNUC__
void __wrap_UART_Handler(void)
#else
void $Sub$$UART_Handler(void)
#endif
{
    uint32_t idd;
    idd = 0x0F & GetWord32(UART_IIR_FCR_REG);
    if(idd!=NO_INT_PEND)
    {
        switch(idd)
        {
          case UART_TIMEOUT:
            if ((uart_sps_env.errordetect == UART_ERROR_DETECT_ENABLED) && uart_fifo_err_getf())
            {
                uart_sps_rec_error_isr();
            }
            
			uart_sps_timeout_data_avail_isr();
            break;
          case RECEIVED_AVAILABLE:
            uart_sps_rec_data_avail_isr();               
            break;

          case THR_EMPTY:
            uart_sps_thr_empty_isr();
            break;

          default:
            break;
        }
    }

}

/// @} UART

