/******************************************************************************
* File Name:   main.c
* 
* Description:
* This file provides example usage of communication protocol self tests for PSoC 4.
*
*  Note that all SCB blocks instances were chosen for this test
*  is based on pin availability on the device's corresponding
*  development kit. The self test demonstrated will work on all instances
*  of the  SCB block.
*
* Related Document:See README.md
*
********************************************************************************
* Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
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
#include <stdio.h>
#include "cy_pdl.h"
#include "cybsp.h"
#include "SelfTest_UART_slave_message.h"
#include "SelfTest_UART_master_message.h"

/*******************************************************************************
* Macros
********************************************************************************/

#define MAX_INDEX_VAL (0xFFF0u)
#define LED_DELAY_MS 50

/*******************************************************************************
* Global Variables
*******************************************************************************/
#define RX_TEST_SIZE        (10u)
#define RX_BUFF_SIZE        (16u)

#define UM_IDLE             (0u)
#define UM_PACKREADY        (1u)
#define UM_RESPOND          (2u)

/* Master states */
#define UM_ERROR               (0u)
#define UM_COMPLETE            (1u)
#define UM_BUSY                (2u)

/* Protocol Timeout (timer ticks) */
#define UM_TIMEOUT             (1000u)



#if COMPONENT_PSOC4000S || COMPONENT_PSOC4000T 
    #define UART_PRINT_ENABLE        0
#else
    #define UART_PRINT_ENABLE        1
    char uart_print_buff[100]={0};
#endif

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void Protocol_Test_UART_Init(void);
static void Timeout_Counter_Init(void);


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* The main function performs the following tasks:
*    1. Initializes the device, board peripherals, and retarget-io for prints.
*    2. Initializes the hardware for UART Protocol test and Setup ISRs
*    3. Initializes the UART Slave and Master Protocol Structures and Enable them
*    4. The loop checks if the master is available for communication.
*    5. If the previous communication resulted in an error, it prints an error
*       message and halts execution, otherwise continue.
*    7. The master sends a new packet of data.
*    8. The code checks if the slave has a ready packet to process and prepares a
*       response and respond if ready.
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
    /* Uart test data */
    uint8_t rxd[RX_BUFF_SIZE];
    uint8_t txd[] = "1234567890ABCDEF";
    uint16_t count = 0u;
    uint8_t slave_resp_res;
    cy_rslt_t result;

    #if UART_PRINT_ENABLE
    cy_stc_scb_uart_context_t CYBSP_UART_context;
    #endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0u);
    }

    /* Enable global interrupts */
    __enable_irq();

    #if UART_PRINT_ENABLE
    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);
    #endif

    /* Init HW for UART Protocol test and Setup ISRs*/
    Timeout_Counter_Init();
    Protocol_Test_UART_Init();

    #if UART_PRINT_ENABLE
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\nClass-B Safety Test: Communication Protocol\r\n");
    #endif

    /* Init UART Slave and Master Protocol Structures and Enable the UART HW */
    UartMesMaster_Init(CYBSP_DUT_UART_MASTER_HW, CYBSP_TIMER_UART_MASTER_HW, CYBSP_TIMER_UART_MASTER_NUM);
    UartMesSlave_Init(CYBSP_DUT_UART_SLAVE_HW, 2u);

    for(;;)
    {
        /*    Master communication process */
        if(UM_BUSY != UartMesMaster_State())
        {

            /* Master is ready for send
             * Show previous communication result */
            if(UM_COMPLETE != UartMesMaster_State())
            {

                /* No respond timeout error */
                if (count > 0u)
                {
                    #if UART_PRINT_ENABLE
                    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\nCommunication Protocol test: error \r\n");
                    #endif
                    /* Set the user LED state */
                    Cy_GPIO_Set(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

                    /* For demo purposes in case of error detection
                    * message is printed to UART Debug and code execution
                    * is stopped here in a while loop */
                    while(1u)
                    {
                    }
                }
            }

            #if UART_PRINT_ENABLE
            /* Print test counter */
            sprintf(uart_print_buff, "\rCommunication Protocol testing at run-time... count=%d", count);
            Cy_SCB_UART_PutString(CYBSP_UART_HW, uart_print_buff);
            #endif
            /* Toggle the user LED state */
            Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

            /* Wait for 0.05 seconds */
            Cy_SysLib_Delay(LED_DELAY_MS);

            /* Send new packet */
            UartMesMaster_DataProc(2u, txd, RX_TEST_SIZE, rxd, sizeof(rxd));
        }

        /* Slave communication process */
        if(UM_PACKREADY == UartMesSlave_State())
        {

            /* Slave have a marker
             * analyze received data and prepare respond */
            slave_resp_res = UartMesSlave_Respond((uint8_t *)UartMesSlave_GetDataPtr(), UartMesSlave_GetDataSize());
            if(slave_resp_res)
            {
                CY_ASSERT(0);
            }
        }

        count++;
        if (count > MAX_INDEX_VAL)
        {
            count = 0u;
        }


        /* Delay emulates user code */
        Cy_SysLib_Delay(50u);
    }
}

/*****************************************************************************
* Function Name: Protocol_Test_UART_Init
******************************************************************************
*
* Summary:
*  Init UART component and hook appropriate ISRs for the background protocol
*  routine.
*
* Parameters:
*  NONE
*
* Return:
*  NONE
******************************************************************************/
static void Protocol_Test_UART_Init(void)
{
    cy_en_sysint_status_t int_result;
    cy_en_scb_uart_status_t uart_result;
    cy_stc_sysint_t intrCfg =
    {
       /*.intrSrc =*/ CYBSP_DUT_UART_MASTER_IRQ, /* Interrupt source is UART interrupt */
       /*.intrPriority =*/ 3UL   /* Interrupt priority is 3 */
    };
    int_result = Cy_SysInt_Init(&intrCfg, UartMesMaster_Msg_ISR);
    if(int_result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0u);
    }
    /* Enable Interrupt */
    NVIC_EnableIRQ(intrCfg.intrSrc);

    /* Reuse intrCfg Struct for Slave UART */
    intrCfg.intrSrc = CYBSP_DUT_UART_SLAVE_IRQ; /* Interrupt source is UART interrupt */
    int_result = Cy_SysInt_Init(&intrCfg, UartMesSlave_Msg_ISR);
    if(int_result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0u);
    }
    /* Enable Interrupt */
    NVIC_EnableIRQ(intrCfg.intrSrc);

    /* Initialize the Master UART */
    uart_result = Cy_SCB_UART_Init(CYBSP_DUT_UART_MASTER_HW, &CYBSP_DUT_UART_MASTER_config, NULL);
    if(uart_result != CY_SCB_UART_SUCCESS)
    {
        CY_ASSERT(0u);
    }

    /* Initialize the Slave UART */
    uart_result = Cy_SCB_UART_Init(CYBSP_DUT_UART_SLAVE_HW, &CYBSP_DUT_UART_SLAVE_config, NULL);
    if(uart_result != CY_SCB_UART_SUCCESS)
    {
        CY_ASSERT(0u);
    }
}

/******************************************************************************
* Function Name: Timeout_Counter_Init
*******************************************************************************
*
* Summary:
* Initialize the Timer interrupt for the UART protocol timeout.
*
* Parameters:
*  NONE
*
* Return:
*  NONE
******************************************************************************/
static void Timeout_Counter_Init(void)
{
    cy_en_sysint_status_t int_result;
    cy_en_tcpwm_status_t tcpwm_result;
    
    cy_stc_sysint_t intrCfg =
    {
       /*.intrSrc =*/ CYBSP_TIMER_UART_MASTER_IRQ, /* Interrupt source is Timer interrupt */
       /*.intrPriority =*/ 3UL   /* Interrupt priority is 3 */
    };

    int_result = Cy_SysInt_Init(&intrCfg, UartMesMaster_Timeout_ISR);

    if(int_result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable Interrupt */
    NVIC_EnableIRQ(intrCfg.intrSrc);

    /* Init and Enable timer */
    tcpwm_result = Cy_TCPWM_Counter_Init(CYBSP_TIMER_UART_MASTER_HW, CYBSP_TIMER_UART_MASTER_NUM, &CYBSP_TIMER_UART_MASTER_config);
    if(tcpwm_result != CY_TCPWM_SUCCESS)
    {
        CY_ASSERT(0);
    }
    Cy_TCPWM_Counter_Enable(CYBSP_TIMER_UART_MASTER_HW, CYBSP_TIMER_UART_MASTER_NUM);
}

/* [] END OF FILE */

