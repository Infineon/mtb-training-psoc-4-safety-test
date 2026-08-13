/******************************************************************************
* File Name:   self_test.c
*
* Description:This file provides the helper functions for executing self tests
*             for  PSoC4 MCU.
*
*
* Related Document: See README.md
*
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
#include <stdlib.h>
#include "cybsp.h"

#include "self_test.h"


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* SelfTest API return status */
uint8_t ret = 0u;
uint16_t count = 0u;

/*Index for IPs*/
uint8_t ip_index = 1u;

/*Flash Checksum for ARM Compiler*/
#if defined (__ARMCC_VERSION)
#if (FLASH_TEST_MODE == FLASH_TEST_FLETCHER64)
static volatile const uint64_t flash_StoredCheckSum __attribute__((used,
                                                                   section(".flash_checksum"))) = 0xFF4E23179ADB8500;
#endif
#if (FLASH_TEST_MODE == FLASH_TEST_CRC32)
static volatile const uint32_t flash_StoredCheckSum __attribute__((used,
                                                                   section(".flash_checksum"))) = 0x8332C4A0UL;
#endif
/*Flash Checksum for IAR Compiler*/
#elif defined (__ICCARM__)
#if (FLASH_TEST_MODE == FLASH_TEST_FLETCHER64)
CY_SECTION(".flash_checksum") const uint64_t flash_StoredCheckSum = 0xD54107E03B4FB8DB;
#endif
#if (FLASH_TEST_MODE == FLASH_TEST_CRC32)
CY_SECTION(".flash_checksum") const uint32_t flash_StoredCheckSum = 0x40BD91C0;
#endif
/*Flash Checksum for GCC_ARM Compiler*/
#else
#if (FLASH_TEST_MODE == FLASH_TEST_FLETCHER64)
static volatile const uint64_t flash_StoredCheckSum __attribute__((used,
                                                                   section(".flash_checksum"))) = 0xF74D3D130607FB4A;
#endif
#if (FLASH_TEST_MODE == FLASH_TEST_CRC32)
static volatile const uint32_t flash_StoredCheckSum __attribute__((used,
                                                                   section(".flash_checksum"))) = 0xBEECF6E0UL;
#endif
#endif

/*****************************************************************************
* Function Name: IAR_Flash_Init
******************************************************************************
* Summary:
* The function ensures that compiler optimizations are not done for IAR
* compiler in the release mode.
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
#if defined (__ICCARM__)
#pragma optimize=none
void IAR_Flash_Init()
{
    SelfTest_Flash_init(CY_FLASH_BASE,FLASH_END_ADDR,flash_StoredCheckSum);
}
#endif

/*****************************************************************************
* Function Name: Clock_Test
******************************************************************************
* Summary:
* Clock Test : Testing clock frequency using Independent Time slot
* monitoring technique
*
* Parameters:
*  initial_run: set to true if this is the first time executing this test.
*
* Return:
*  void
*****************************************************************************/
void Clock_Test(bool initial_run)
{
    if (INITIAL_TEST == initial_run)
    {
        /* Initialize WDT and Timer for Clock Self test */
        clock_test_init();
    }

    for (;;)
    {
        ret = SelfTest_Clock(CYBSP_CLOCK_TEST_TIMER_HW, CYBSP_CLOCK_TEST_TIMER_NUM);
        print_result(ip_index,"Clock Test", ret);

        if (PASS_STILL_TESTING_STATUS != ret) {
            break;
        }

        count++;
        if (count > MAX_INDEX_VAL){
            count = 0u;
        }
    }
    Cy_SysLib_ClearResetReason();
    /* Either you need to clear WDT interrupt periodically or
     * disable it to ensure no WDT reset */
    Cy_WDT_ClearInterrupt();
    Cy_WDT_Disable();
    ip_index++;
}


/*****************************************************************************
* Function Name: Stack_Memory_Test
******************************************************************************
* Summary:
* Memory Test: Testing memory using Self Tests...
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void Stack_Memory_Test(void)
{
    /* Init Stack SelfTest */
    SelfTests_Init_Stack_Range((uint16_t*)DEVICE_STACK_BASE, DEVICE_STACK_SIZE, PATTERN_BLOCK_SIZE);

    /*******************************/
    /* Run Stack Self Test...      */
    /*******************************/
    uint8_t ret = SelfTests_Stack_Check_Range((uint16_t*)DEVICE_STACK_BASE, DEVICE_STACK_SIZE);
    if ((ERROR_STACK_OVERFLOW & ret))
    {
         /* Process error */
        print_result(ip_index,"Stack Overflow Test", ret);
    }
    else if ((ERROR_STACK_UNDERFLOW & ret))
    {
        ret = ERROR_STATUS;
         /* Process error */
        print_result(ip_index,"Stack Underflow Test", ret);
    }

    else
    {
        print_result(ip_index,"Stack Memory Test", ret);
    }
    ip_index++;
}

/*****************************************************************************
* Function Name: Flash_Test
******************************************************************************
* Summary:
* Flash Test : Testing the flash by comparing the stored checksum in
* flash with calculated checksum of the data stored in flash.
*
* Parameters:
*  initial_run: set to true if this is the first time executing this test. 
*
* Return:
*  void
*****************************************************************************/
void Flash_Test(bool initial_run)
{
    /* Variable for output calculated Flash Checksum */
    uint8_t flash_CheckSum_temp;
    char flash_CheckSum_act[32]={0};

    if (INITIAL_TEST == initial_run)
    {
#if defined (__ICCARM__)
        IAR_Flash_Init();
#else
        SelfTest_Flash_init(CY_FLASH_BASE,FLASH_END_ADDR,flash_StoredCheckSum);
#endif
    }

    for(;;)
    {
        ret = SelfTest_FlashCheckSum(FLASH_DOUBLE_WORDS_TO_TEST);

        print_result(ip_index, "Flash Test", ret);

        if (ERROR_STATUS == ret)
        {
#if(FLASH_TEST_MODE == FLASH_TEST_CRC32)
            Cy_SCB_UART_PutString(CYBSP_UART_HW,"\r\nFLASH CRC: 0x");
#elif (FLASH_TEST_MODE == FLASH_TEST_FLETCHER64)
            Cy_SCB_UART_PutString(CYBSP_UART_HW,"\r\nFLASH CHECKSUM: 0x");
#endif

            /* Output calculated Flash Checksum */
            for(int16_t i = sizeof(flash_CheckSum) - 1; i >= 0; i--)
            {
                flash_CheckSum_temp = (uint8_t) (flash_CheckSum >> (i*8u));
                sprintf(flash_CheckSum_act, "%02X", flash_CheckSum_temp);
                Cy_SCB_UART_PutString(CYBSP_UART_HW, flash_CheckSum_act);
            }
            Cy_SCB_UART_PutString(CYBSP_UART_HW,"\r\n");
            break;
        }
        else if (PASS_COMPLETE_STATUS == ret) {
            break;
        }
        else
        {
            /* Do Nothing */
        }
        count++;
        if (count > MAX_INDEX_VAL) {
            count = 0u;
        }
    }
    ip_index++;
}

#if DMA_ENABLE
/*****************************************************************************
* Function Name: DMAC_Test
******************************************************************************
* Summary:
* DMAC Test : The function performs selftest for DMA IP.
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void DMAC_Test(void)
{
    /**********************************/
    /* Run DMAC Self Test... */
    /**********************************/
    ret = SelfTest_DMAC(DMAC, 0UL, TRIG0_OUT_CPUSS_DMAC_TR_IN0);

    print_result(ip_index++, "DMAC Test", ret);
}
#endif

/*****************************************************************************
* Function Name: Interrupt_Test
******************************************************************************
* Summary:
* Interrupt Test : Testing Interrupt controller using independent time
* slot monitoring technique
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void Interrupt_Test(void)
{
    Interrupt_test_Init();

    ret = SelfTest_Interrupt(CYBSP_TIMER_HW, CYBSP_TIMER_NUM);
    print_result(ip_index++,"Interrupt Test", ret);
    if(ERROR_STATUS == ret)
    {
        Cy_SCB_UART_PutString(CYBSP_UART_HW,"\r\n");
    }
}


/*****************************************************************************
* Function Name: IO_Test
******************************************************************************
* Summary:
* IO Test : Testing IO functionality by writing 1/0 to each pin and
* then reading it back.
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void IO_Test(void)
{
    ret = SelfTest_IO();
    if (OK_STATUS != ret)
    {
        ret = ERROR_STATUS;
    }
    print_result(ip_index++,"GPIO Test",ret);

}


/*****************************************************************************
* Function Name: SRAM_March_Test
******************************************************************************
* Summary:
* SRAM March Test: Testing SRAM using March Self Tests...
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void SRAM_March_Test(void)
{
    uint8_t restore_buff[BUFFER_SIZE] = {0u};

    __disable_irq();

    ret = SelfTest_SRAM(SRAM_MARCH_TEST_MODE,(uint8_t *)DEVICE_SRAM_BASE,BLOCK_SIZE,(uint8_t *)restore_buff,BUFFER_SIZE);

    __enable_irq();

    print_result(ip_index,"SRAM March Test", ret);
    ip_index++;

}

/*****************************************************************************
* Function Name: Stack_March_Test
******************************************************************************
* Summary:
* Stack March Test: Testing Stack using March Self Tests.
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void Stack_March_Test(void)
{
    __disable_irq();

    ret = SelfTest_SRAM_Stack((uint8_t *)DEVICE_STACK_BASE,(uint32_t)DEVICE_STACK_SIZE,
            (uint8_t *)STACK_TEST_BUFF+ (uint32_t)DEVICE_STACK_SIZE);
    __enable_irq();

     /*Process error*/
    print_result(ip_index,"Stack March Test", ret);
    ip_index++;
}


/******************************************************************************
* Function Name: Interrupt_test_Init
*******************************************************************************
*
* Summary: Initialize the Timer interrupt for Interrupt Self test.
*
* Parameters:
*  void
*
* Return:
*  void
*
******************************************************************************/
void Interrupt_test_Init(void)
{
    cy_rslt_t result;
    cy_stc_sysint_t intrCfg =
    {
       /*.intrSrc =*/ CYBSP_TIMER_IRQ, /* Interrupt source is Timer interrupt */
       /*.intrPriority =*/ 3UL   /* Interrupt priority is 3 */
    };

    result = Cy_SysInt_Init(&intrCfg, SelfTest_Interrupt_ISR_TIMER);

    if(result != CY_SYSINT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable Interrupt */
    NVIC_EnableIRQ(intrCfg.intrSrc);


    /* Initialize TCPWM counter*/
    result = Cy_TCPWM_Counter_Init(CYBSP_TIMER_HW, CYBSP_TIMER_NUM, &CYBSP_TIMER_config);
    if(result != CY_TCPWM_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable timer */
    Cy_TCPWM_Counter_Enable(CYBSP_TIMER_HW, CYBSP_TIMER_NUM);

    Cy_TCPWM_SetInterruptMask(CYBSP_TIMER_HW, CYBSP_TIMER_NUM, CY_TCPWM_INT_ON_TC);

}

/*****************************************************************************
* Function Name: clock_test_init
******************************************************************************
* Summary:
* This function initializes the WDT block and initialize the timer interrupt
* for the Self test.
*
* Parameters:
*  void
*
* Return:
*  void
*
*****************************************************************************/
void clock_test_init(void)
{
    cy_en_tcpwm_status_t tcpwm_res;
    cy_en_sysint_status_t sysint_res;

    /* Write the ignore bits - operate with full 16 bits */
    Cy_WDT_SetIgnoreBits(IGNORE_BITS);

    if(Cy_WDT_GetIgnoreBits() != IGNORE_BITS)
    {
        CY_ASSERT(0);
    }

    /* Clear match event interrupt, if any */
    Cy_WDT_ClearInterrupt();

    /* Enable ILO */
    Cy_SysClk_IloEnable();

    /* Waiting for proper start-up of ILO */
    Cy_SysLib_Delay(ILO_START_UP_TIME);

    Cy_WDT_Enable();
    if(Cy_WDT_IsEnabled() == false)
    {
        CY_ASSERT(0);
    }

    tcpwm_res = Cy_TCPWM_Counter_Init(CYBSP_CLOCK_TEST_TIMER_HW, CYBSP_CLOCK_TEST_TIMER_NUM, &CYBSP_CLOCK_TEST_TIMER_config);
    if(CY_TCPWM_SUCCESS != tcpwm_res)
    {
        CY_ASSERT(0);
    }

    cy_stc_sysint_t intrCfg =
    {
       /*.intrSrc =*/ CYBSP_CLOCK_TEST_TIMER_IRQ, /* Interrupt source is Timer interrupt */
       /*.intrPriority =*/ 3UL   /* Interrupt priority is 3 */
    };

    sysint_res = Cy_SysInt_Init(&intrCfg, SelfTest_Clock_ISR_TIMER);

    if(CY_SYSINT_SUCCESS != sysint_res)
    {
        CY_ASSERT(0);
    }

    /* Enable Interrupt */
    NVIC_EnableIRQ(intrCfg.intrSrc);

    /* Enable timer */
    Cy_TCPWM_Counter_Enable(CYBSP_CLOCK_TEST_TIMER_HW, CYBSP_CLOCK_TEST_TIMER_NUM);

    Cy_TCPWM_SetInterruptMask(CYBSP_CLOCK_TEST_TIMER_HW, CYBSP_CLOCK_TEST_TIMER_NUM, CY_TCPWM_INT_ON_TC);

}

/*****************************************************************************
* Function Name: Start_Up_Test
******************************************************************************
* Summary:
* Start Up Test : This function checks the startup configuration registers.
*
* Parameters:
*  initial_run: set to true if this is the first time executing this test. 
*
* Return:
*  void
*****************************************************************************/
void Start_Up_Test(bool initial_run)
{
	if (INITIAL_TEST == initial_run)
	{
#if COMPONENT_CAT1A
	    /* This function initilizes the AREF address depending on the device.*/
	    SelfTests_Init_StartUp_ConfigReg();
#endif

#if (STARTUP_CFG_REGS_MODE == CFG_REGS_TO_FLASH_MODE)

	    /*******************************/
	    /* Save Start-Up registers...  */
	    /*******************************/
	    if (CY_FLASH_DRV_SUCCESS  != SelfTests_Save_StartUp_ConfigReg())
	    {
	        /* Process error */
	        Cy_SCB_UART_PutString(CYBSP_UART_HW,"Error: Can't save Start-Up Config Registers\r\n");
	    }

#endif /* End (STARTUP_CFG_REGS_MODE == CFG_REGS_TO_FLASH_MODE) */
	}
    /**********************************/
    /* Run Start-Up regs Self Test... */
    /**********************************/
    ret = SelfTests_StartUp_ConfigReg();

    /* Process error */
    print_result(ip_index++,"Start-Up Register",ret);

}

/*****************************************************************************
* Function Name: print_result
******************************************************************************
* Summary:
* This function displays results on the UART terminal based on the return value
* from the selftests, which is passed as a one of the parameter to this function.
*
* Parameters:
*  void
*
* Return:
*  void
*****************************************************************************/
void print_result(uint8_t index, char* test_name, uint8_t ret)
{
    char result_buff[100];
    uint8_t count=10;

    do {
        if (OK_STATUS == ret) {
            /* Process success */
            sprintf(result_buff, "| %-4d| %-32s| %-12s|\r\n", index, test_name, "SUCCESS");
            Cy_SCB_UART_PutString(CYBSP_UART_HW,result_buff);
        }
        else if (PASS_COMPLETE_STATUS == ret) {
            /* Process status */ \
            sprintf(result_buff, "| %-4d| %-32s| %-12s|\r\n", index, test_name, "SUCCESS");
            Cy_SCB_UART_PutString(CYBSP_UART_HW,result_buff);
            break;
        }
        else if (PASS_STILL_TESTING_STATUS == ret) {
            /* Print test counter */
            sprintf(result_buff, "| %-4d| %-32s| %-12s|count=%d\r", index, test_name, "IN PROGRESS",count);
            Cy_SCB_UART_PutString(CYBSP_UART_HW,result_buff);
        }
        else {
            /* Process error */
            sprintf(result_buff, "| %-4d| %-32s| %-12s|\r\n", index, test_name, "ERROR");
            Cy_SCB_UART_PutString(CYBSP_UART_HW,result_buff);
        }
    } while (0);

}

/* [] END OF FILE */
