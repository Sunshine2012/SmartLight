/**
****************************************************************************************
*
* @file battery.c
*
* @brief Battery driver. Provides Battery level. Uses ADC module to get current voltage.
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
#include "adc.h"
#include "battery.h"

/**
 ****************************************************************************************
 * ADC module Functions
 ****************************************************************************************
*/

/**
 ****************************************************************************************
 * @brief Calculates battery level percentage for CR2032 batteries
 *
 * @param[in] adc_sample  adc sample 
 *
 * @return Battery level. 0 - 100
 ****************************************************************************************
 */
 
 uint16_t dbg_adc;
 
uint8_t batt_cal_cr2032(uint16_t adc_sample)
{
    uint8_t batt_lvl;
    
    dbg_adc = adc_sample;
    if (adc_sample > 1705)
		batt_lvl = 100;
	else if (adc_sample <= 1705 && adc_sample > 1584) 
		batt_lvl = 28 + (uint8_t)(((float)((float)(adc_sample - 1584)/(float)(1705 - 1584))) * 72) ;
	else if (adc_sample <= 1584 && adc_sample > 1360) 
		batt_lvl = 4 + (uint8_t)(((float)((float)(adc_sample - 1360)/(float)(1584 - 1360))) * 24) ;
	else if (adc_sample <= 1360 && adc_sample > 1136) 
		batt_lvl = (uint8_t)(((float)((float)(adc_sample - 1136)/(float)(1360 - 1136))) * 4) ;
	else 
		batt_lvl = 0;
		
	return batt_lvl;	
}

/**
 ****************************************************************************************
 * @brief Calculates battery level percentage for CR2032 batteries
 *
 * @param[in] adc_sample  adc sample 
 *
 * @return Battery level. 0 - 100
 ****************************************************************************************
 */
uint8_t batt_cal_cr1225(uint16_t adc_sample)
{
    uint8_t batt_lvl;
    
    //1705=3.0V, 1137=2V
    if(adc_sample >= 1137)
        batt_lvl = (adc_sample - 1137)*100/568;
    else
        batt_lvl = 0;
    
    return batt_lvl;
}

/**
 ****************************************************************************************
 * @brief Calculates battery level percentage for a single AAA battery.
 *  
 *
 * @param[in] adc_sample  adc sample 
 *
 * @return Battery level. 0 - 100
 ****************************************************************************************
 */
uint8_t batt_cal_aaa(uint16_t adc_sample)
{
    uint8_t batt_lvl;
    
    //1,50V = 0x340    
    //1,00V = 0x230
    //0,90V = 0x1F0    
    //0,80V = 0x1B0
    if(adc_sample >= 0x1B0)
        batt_lvl = (adc_sample - 0x1B0)*100/(0x340-0x1B0);
    else
        batt_lvl = 0;
    
    return batt_lvl;
}


/**
 ****************************************************************************************
 * @brief Reads current voltage from adc module and returns battery level. 
 *
 * @param[in] batt_type     Battery type. Supported types defined in battery.h
 *
 * @return Battery level. 0 - 100%
 ****************************************************************************************
 */

uint8_t battery_get_lvl(uint8_t batt_type)
{
	uint8_t batt_lvl;
	uint16_t adc_sample;
	volatile int i;

	adc_init(GP_ADC_SE, GP_ADC_SIGN);
	
	if (batt_type == BATT_AAA_SINGLE_ALKALINE)
		adc_enable_channel(ADC_CHANNEL_VBAT1V);
	else
		adc_enable_channel(ADC_CHANNEL_VBAT3V);
	
    adc_sample = adc_get_sample();

	adc_init(GP_ADC_SE, 0);
    
	if (batt_type == BATT_AAA_SINGLE_ALKALINE)
		adc_enable_channel(ADC_CHANNEL_VBAT1V);
	else
		adc_enable_channel(ADC_CHANNEL_VBAT3V);

	adc_sample += adc_get_sample();
	
	adc_disable();
	
    adc_sample >>= 4;
    adc_sample <<= 4;
      
    switch (batt_type)
    {
        case BATT_CR2032:
            batt_lvl = batt_cal_cr2032(adc_sample);
            break;
        case BATT_CR1225:
            batt_lvl = batt_cal_cr1225(adc_sample);
            break;
        case BATT_AAA_SINGLE_ALKALINE:
            batt_lvl = batt_cal_aaa(adc_sample);
            break;
        default:
            batt_lvl = 0;
    }   
    
	return batt_lvl;
}
