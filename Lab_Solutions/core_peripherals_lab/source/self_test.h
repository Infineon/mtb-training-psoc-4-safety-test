/******************************************************************************
* File Name:   self_test.h
*
* Description: This file is the public interface of self_test.c source file.
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

/*******************************************************************************
* Include guard
*******************************************************************************/
#ifndef SELF_TEST_H_
#define SELF_TEST_H_
#include <stdio.h>
#include "SelfTest.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/
extern uint16_t count;
/*Index for IPs*/
extern uint8_t ip_index;
/* SelfTest API return status */
extern uint8_t ret;

/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_INDEX_VAL (0xFFF0u)

/*The size of RAM/ STACK block to be tested. */
#define BLOCK_SIZE                1024
/*The size of buffer which is used to store/restore. */
#define BUFFER_SIZE               1024
#define MAX_INDEX_VAL (0xFFF0u)
#define PATTERN_BLOCK_SIZE (8u)
#define DEVICE_SRAM_BASE     (0x20000000)
#define DEVICE_STACK_SIZE    (0x00000400)
#define STACK_TEST_BUFF      (0x20000500)

#if COMPONENT_PSOC4100SMAX || COMPONENT_PSOC4100SP256KB || COMPONENT_PSOC4100TP
#define DEVICE_SRAM_SIZE     (0x00008000)
#endif
#if COMPONENT_PSOC4100SP
#define DEVICE_SRAM_SIZE     (0x00004000)
#endif
#if COMPONENT_PSOC4000S 
#define DEVICE_SRAM_SIZE     (0x00001000)
#endif
#if COMPONENT_PSOC4000T || COMPONENT_PSOC4100S
#define DEVICE_SRAM_SIZE     (0x00002000)
#endif

#define DEVICE_STACK_BASE    (DEVICE_SRAM_BASE + DEVICE_SRAM_SIZE)

#if COMPONENT_PSOC4100S || COMPONENT_PSOC4000S || COMPONENT_PSOC4000T
#define DMA_ENABLE    0
#else
#define DMA_ENABLE    1
#endif
#define INITIAL_TEST 		(true)
#define RUNTIME_TEST		(false)

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Clock_Test(bool initial_run);
void Flash_Test(bool initial_run);

void Interrupt_Test(void);
void IO_Test(void);
void SRAM_March_Test(void);
void Stack_March_Test(void);
void Stack_Memory_Test(void);
void Interrupt_test_Init(void);
void clock_test_init(void);
void Start_Up_Test(bool initial_run);
#if DMA_ENABLE
void DMAC_Test(void);
#endif
void print_result(uint8_t, char*, uint8_t);


#endif /* SELF_TEST_H_ */


/* [] END OF FILE */
