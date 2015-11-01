/**
 ****************************************************************************************
 *
 * @file periph_setup.c
 *
 * @brief Peripherals setup and initialization. 
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
#include "rwip_config.h"             // SW configuration
#include "periph_setup.h"            // peripheral configuration
#include "global_io.h"
#include "gpio.h"
#include "uart_sps.h"                    // UART initialization
#include "pwm.h"

extern timer0_handler_function_t* TIMER0_callback;

/**
 ****************************************************************************************
 * @brief Each application reserves its own GPIOs here.
 *
 * @return void
 ****************************************************************************************
 */



// 
void timer_init()
{
    // 定时时钟使能
    set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
    //set_tmr_div(CLK_PER_REG_TMR_DIV_1);
    // PWM_MODE_CLOCK_DIV_BY_TWO    把系统时钟分为两部分，1~8MHz
    //timer0_init(TIM0_CLK_FAST,  PWM_MODE_CLOCK_DIV_BY_TWO, TIM0_CLK_DIV_BY_10);
    timer0_init(TIM0_CLK_FAST,  PWM_MODE_ONE, TIM0_CLK_NO_DIV);
	//timer0_set_pwm_on_counter(2000);
	timer0_set_pwm_high_counter(6000);
	timer0_set_pwm_low_counter(10000);
    //timer0_set(0, 400, 1600);
    timer0_enable_irq();
}

void timer_callback()
{
    static unsigned int flag1 = 0;
    static unsigned int flag2 = 0;
    static unsigned int count = 32;
    flag1++;
    if(10 == flag1)
    {
        // 呼吸灯的功能
    	flag2++;
    	if(flag2 < 250)
    	{
    		count = (count + 32);
    		
    	}
    	else if (flag2 >= 250 && flag2 < 500)
    	{
    		count = (count - 32);	
    	}
    	else 
    	{
    	    flag2 = 0;
    	    count = 32;
    	}
    	
		timer0_set_pwm_high_counter(count);
        timer0_set_pwm_low_counter(16000-count);
    	SetWord16(P1_DATA_REG,~(GetWord16(P1_DATA_REG)) | 0xfd);
        //GPIO_SetActive(GPIO_PORT_1, GPIO_PIN_0);
        //timer0_set_pwm_high_counter();
        flag1 = 0;
        
    }


    
}

#if DEVELOPMENT_DEBUG   // aiwesky 20150927

void GPIO_reservations(void)
{
/*
* Globally reserved GPIOs reservation
*/

/*
* Application specific GPIOs reservation. Used only in Development mode (#if DEVELOPMENT_DEBUG)
    
i.e.  
    RESERVE_GPIO(DESCRIPTIVE_NAME, GPIO_PORT_0, GPIO_PIN_1, PID_GPIO);    //Reserve P_01 as Generic Purpose I/O
*/
    
    RESERVE_GPIO( UART1_TX, GPIO_PORT_0, GPIO_PIN_4, PID_UART1_TX);  
    RESERVE_GPIO( UART1_RX, GPIO_PORT_0, GPIO_PIN_5, PID_UART1_RX);
    RESERVE_GPIO( LED, GPIO_PORT_1, GPIO_PIN_0, PID_GPIO);
    //RESERVE_GPIO( LED, GPIO_PORT_1, GPIO_PIN_0, PID_PWM0);
}

#endif //DEVELOPMENT_DEBUG

/**
 ****************************************************************************************
 * @brief Map port pins
 *
 * The Uart and SPI port pins and GPIO ports are mapped
 ****************************************************************************************
 */
void set_pad_functions(void)        // set gpio port function mode
{

#if DEVELOPMENT_DEBUG   // aiwesky 20150927
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_4, OUTPUT, PID_UART1_TX, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_5, INPUT, PID_UART1_RX, false );
#endif //DEVELOPMENT_DEBUG
	
	
#if (UART_HW_FLOW_ENABLED) 
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_3, OUTPUT, PID_UART1_RTSN, false );
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_2, INPUT, PID_UART1_CTSN, false );
#endif /*UART_HW_FLOW_ENABLED*/
    
    GPIO_ConfigurePin( GPIO_PORT_1, GPIO_PIN_0, OUTPUT, PID_GPIO, false );
    
/*
* Configure application ports.
i.e.    
    GPIO_ConfigurePin( GPIO_PORT_0, GPIO_PIN_1, OUTPUT, PID_GPIO, false ); // Set P_01 as Generic purpose Output
*/
}


/**
 ****************************************************************************************
 * @brief Enable pad's and peripheral clocks assuming that peripherals' power domain is down. The Uart and SPi clocks are set.
 *
 * @return void
 ****************************************************************************************
 */
void periph_init(void) 
{
	// Power up peripherals' power domain
    SetBits16(PMU_CTRL_REG, PERIPH_SLEEP, 0);
    while (!(GetWord16(SYS_STAT_REG) & PER_IS_UP)) ; 
    
	
		//rom patch
		patch_func();
	
		//Init pads
		set_pad_functions();

    SetBits16(CLK_PER_REG, UART1_ENABLE, 1);    // enable clock - always @16MHz

    // baudr=9-> 115k2
    // mode=3-> no parity, 1 stop bit 8 data length
	
#if DEVELOPMENT_DEBUG   // aiwesky 20150927
    uart_sps_init(UART_SPS_BAUDRATE, 3); //exact baud rate defined in uart.h
#endif //DEVELOPMENT_DEBUG
	
    // aiwesky 20150530
    timer_init();
    
    TIMER0_callback = timer_callback;
    timer0_register_callback(timer_callback);
    
		// Enable the pads
		SetBits16(SYS_CTRL_REG, PAD_LATCH_EN, 1);
}
