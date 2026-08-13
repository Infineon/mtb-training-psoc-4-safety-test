/******************************************************************************
* File Name: main.c
*
* Description:This file provides the following core peripheral self tests
*             for PSoC 4 :
*             - CPU registers test
*             - Program Counter test
*             - Program Flow test
*             - WDT test
*             - Clock test
*             - Interrupt test
*             - IO test
*             - Flash test (fletcher's test + CRC test)
*             - Config Registers test
*             - SRAM/Stack test (March test)
*             - Stack Overflow test
*             - DMAC
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
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
*******************************************************************************/

/*******************************************************************************
* Includes
********************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include "self_test.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CY_ASSERT_FAILED          (0u)

/*******************************************************************************
* Global Variables
*******************************************************************************/

uint32_t test_count = 0;

static void print_self_test_header(void)
{
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    Cy_SCB_UART_PutString(CYBSP_UART_HW,"\x1b[2J\x1b[;H");

    Cy_SCB_UART_PutString(CYBSP_UART_HW,"*************** "
           "Class-B Safety Test for PSOC 4: Core Peripheral Resources "
           "************** \r\n\n");

    Cy_SCB_UART_PutString(CYBSP_UART_HW,"------------------------------------------------------- \r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW,"| #   | IP under test                   | Test Status | \r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW,"------------------------------------------------------- \r\n");
}

static void print_self_test_footer(void)
{
    char uart_disp_buff[64]={0};

    Cy_SCB_UART_PutString(CYBSP_UART_HW,"------------------------------------------------------- \r\n\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW,"END of the Core CPU Test.\r\n\n");
    sprintf(uart_disp_buff,"Total number of IPs covered in the Test      %d\r\n", --ip_index);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, uart_disp_buff);

    sprintf(uart_disp_buff, "Test Count %d ", test_count);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, uart_disp_buff);
}

static void run_initial_self_test(void)
{
    print_self_test_header();

    ip_index = 0;
    /* Start Up Test */
    Start_Up_Test(INITIAL_TEST);

    /* Program counter Test */
    ret = SelfTest_PC();
    print_result(ip_index++,"Program Counter Test",ret);

    /* CPU Registers Test*/
    ret = SelfTest_CPU_Registers();
    print_result(ip_index++,"CPU Register Test", ret);

    /* Program Flow Test*/
    ret = SelfTest_PROGRAM_FLOW();
    print_result(ip_index++,"Program Flow Test", ret);

    /* Watch Dog Timer Test */
    ret = SelfTest_WDT();
    print_result(ip_index++,"Watchdog Test", ret);

    /* GPIO Test */
    IO_Test();

    #if DMA_ENABLE
    /* DMAC Test */
    DMAC_Test();
    #endif

   /* Clock Test */
    Clock_Test(INITIAL_TEST);

    /* Interrupt Test */
    Interrupt_Test();

    /* SRAM Test */
    SRAM_March_Test();

    /* STACK Test */
    Stack_March_Test();

    /* Stack Overflow and Underflow Test */
    Stack_Memory_Test();

    /* Flash Test */
    Flash_Test(INITIAL_TEST);

    test_count++;
    print_self_test_footer();
	
}

static void run_runtime_self_tests(void)
{
    print_self_test_header();

    ip_index = 0;
    /* Start Up Test */
    Start_Up_Test(RUNTIME_TEST);

    /* Program counter Test */
    ret = SelfTest_PC();
    print_result(ip_index++,"Program Counter Test",ret);

    /* CPU Registers Test*/
    ret = SelfTest_CPU_Registers();
    print_result(ip_index++,"CPU Register Test", ret);

    /* Program Flow Test*/
    ret = SelfTest_PROGRAM_FLOW();
    print_result(ip_index++,"Program Flow Test", ret);

    /* GPIO Test */
    IO_Test();

    #if DMA_ENABLE
    /* DMAC Test */
    DMAC_Test();
    #endif

   /* Clock Test */
    Clock_Test(RUNTIME_TEST);

    /* Interrupt Test */
    Interrupt_Test();

    /* SRAM Test */
    SRAM_March_Test();

    /* STACK Test */
    Stack_March_Test();

    /* Stack Overflow and Underflow Test */
    Stack_Memory_Test();

    /* Flash Test */
    Flash_Test(RUNTIME_TEST);

    test_count++;
    print_self_test_footer();
}



/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. It does...
*    1. Initialize the device and board peripherals and retarget-io for prints
*    2. Calls the test APIs for testing the followings:
*        - Program Counter
*        - Program Flow test
*        - CPU registers
*        - WDT
*        - Clock
*        - Interrupt
*        - Flash
*        - IO
*        - Config Registers test
*        - SRAM/Stack test (March test)
*        - Stack Overflow test
*        - DMAC
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

    cy_stc_scb_uart_context_t CYBSP_UART_context;

    char uart_disp_buff[64]={0};

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Configure and enable the UART peripheral */
    Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    Cy_SCB_UART_Enable(CYBSP_UART_HW);

    /* Enable global interrupts */
    __enable_irq();

    run_initial_self_test();

    
    for (;;)
    {
        run_runtime_self_tests();
        /* Delay emulates user code */
        Cy_SysLib_Delay(5000u);

    }

}

/* [] END OF FILE */
