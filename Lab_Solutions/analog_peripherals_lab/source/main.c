/******************************************************************************
* File Name:   main.c
* 
* Description:
* This file provides example usage of Analog self tests for PSoC 4.
*
* Note that all hardware blocks instances and routing were chosen for this
* test is based on pin availability on the device's corresponding
* development kit. The self test demonstrated will work on all instances
* of the hardware block.
*
* Related Document:See README.md
*
********************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
********************************************************************************/

/*******************************************************************************
* Includes
********************************************************************************/
#include "cy_pdl.h"
#include "cy_systick.h"
#include "cybsp.h"
#include "self_test.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define ADC_TEST_CHANNEL					(0U)
#define ADC_MEASUREMENT_CHANNEL				(1U)

#define ADC_TEST_CHANNEL_MASK				(1<<ADC_TEST_CHANNEL)
#define ADC_MEASUREMENT_CHANNEL_MASK		(1<<ADC_MEASUREMENT_CHANNEL)

/** ADC conversion time, uS */
#define ADC_CON_TIME_uS             (1000u)

#define ONE_SECOND_TICK				(1000u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile uint32_t systick_count = 0;
/*******************************************************************************
* Function Prototypes
*******************************************************************************/

void print_available_commands(void)
{
	/* Display available commands */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "Available commands \r\n");
    #if ADC_ENABLE
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "1 : Run SelfTest for ADC\r\n");
    #endif
    #if COMPARATOR_ENABLE
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "2 : Run SelfTest for Comparator\r\n");
    #endif
    #if OPAMP_ENABLE
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "3 : Run SelfTest for OP-AMP\r\n\n");
    #endif
}

void SysTick_Callback(void)
{
	systick_count++;
}

void scan_measure_channel(void)
{
    int16_t adc_res = 0;

    uint16_t guardCnt;
    /* Start ADC conversion */
    Cy_SAR_StartConvert(CYBSP_DUT_SAR_ADC_HW, CY_SAR_START_CONVERT_SINGLE_SHOT);

    /* Wait for the end of conversion using a guard interval > ADC conversion time */
    guardCnt = 0u;
    do
    {
        guardCnt++;
        Cy_SysLib_DelayUs(1u);
    } while((Cy_SAR_IsEndConversion(CYBSP_DUT_SAR_ADC_HW,
                                    CY_SAR_RETURN_STATUS) != CY_SAR_SUCCESS) &&
            (guardCnt < ADC_TEST_CON_TIME_uS));

    /* Check if timeout */
    if (guardCnt < ADC_CON_TIME_uS)
    {
		char adc_string[100]; 
        /* Read the value from ADC */
        adc_res = Cy_SAR_GetResult16(CYBSP_DUT_SAR_ADC_HW, ADC_MEASUREMENT_CHANNEL);
        adc_res = Cy_SAR_CountsTo_mVolts(CYBSP_DUT_SAR_ADC_HW, ADC_MEASUREMENT_CHANNEL, adc_res);

		sprintf(adc_string, "ADC Channel %d Result: %hd mV\r\n", ADC_MEASUREMENT_CHANNEL, adc_res);
		Cy_SCB_UART_PutString(CYBSP_UART_HW, adc_string);
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* The main function performs the following tasks:
*    1. Initializes the device, board peripherals, and retarget-io for prints.
*    2. Save user SARSEQ ADC routing and Initialize the ADC and AMUXBUS 
*    3. configure AMUXBUS routing and Connect VREF as per the selected test modes
*    4. Run Comparator test
*    5. Run ADC test
*    6. Run OpAmp test
*    7. The code restores the user's SAR ADC configurations
*    
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    cy_rslt_t result;
    #if ADC_ENABLE
    cy_en_sar_status_t sar_res;
    #endif
    cy_stc_scb_uart_context_t CYBSP_UART_context;
    static volatile uint8_t valid_command = 0;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

	Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_CPU, ((uint32_t)(((1000)/1000000.0)*48000000)));
	/* Register one of the SysTick callbacks */
    Cy_SysTick_SetCallback(0UL, &SysTick_Callback);
	Cy_SysTick_Enable();
    
    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\nClass B Safety test: Analog Peripherals\r\n");

    #if ADC_ENABLE
    /* Initialize ADC as per config structure and enable ADC */
    sar_res = Cy_SAR_Init(CYBSP_DUT_SAR_ADC_HW, &CYBSP_DUT_SAR_ADC_config);
    if (sar_res != CY_SAR_SUCCESS)
    {
        CY_ASSERT(0);
    }   
    Cy_SAR_Enable(CYBSP_DUT_SAR_ADC_HW);
    #endif

	Cy_SAR_DisableChannels(CYBSP_DUT_SAR_ADC_HW, ADC_TEST_CHANNEL_MASK | ADC_MEASUREMENT_CHANNEL_MASK);

    print_available_commands();

    for(;;)
    {
		if (ONE_SECOND_TICK < systick_count)
		{
			systick_count = 0;
			Cy_SAR_EnableChannels(CYBSP_DUT_SAR_ADC_HW, ADC_MEASUREMENT_CHANNEL_MASK);
			scan_measure_channel();
			Cy_SAR_DisableChannels(CYBSP_DUT_SAR_ADC_HW, ADC_TEST_CHANNEL_MASK | 
														 ADC_MEASUREMENT_CHANNEL_MASK);
		}
        result = Cy_SCB_UART_Get(CYBSP_UART_HW);
        if (result != CY_SCB_UART_RX_NO_DATA)
        {
            #if ADC_ENABLE
            if (SELFTEST_CMD_ADC == result)
            {
                valid_command++;
                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n[Command] : Run SelfTest for ADC\r\n");
				Cy_SAR_EnableChannels(CYBSP_DUT_SAR_ADC_HW, ADC_TEST_CHANNEL_MASK);
                adc_test();
				Cy_SAR_DisableChannels(CYBSP_DUT_SAR_ADC_HW, ADC_TEST_CHANNEL_MASK | 
															 ADC_MEASUREMENT_CHANNEL_MASK);
            }
            #endif
            #if COMPARATOR_ENABLE
            if (SELFTEST_COMPARATOR == result)
            {
                valid_command++;
                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n[Command] : Run SelfTest for Comparator\r\n");
                comparator_test();
            }
            #endif
            #if OPAMP_ENABLE
            if (SELFTEST_CMD_OPAMP == result)
            {
                valid_command++;
                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n[Command] : Run SelfTest for OP-AMP\r\n");
                opamp_test();
            }
            #endif
            if(valid_command == 0)
            {
                Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\nEnter a valid command\r\n");
            }
            valid_command = 0;

			print_available_commands();
        }

		
		
    }

}

/* [] END OF FILE */
