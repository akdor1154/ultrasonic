/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

CY_ISR_PROTO(onUltrasonicReceived);
CY_ISR(onUltrasonicReceived) {
    ledReg_Write(0xFF);
}

int main() {
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    ultrasonicReset_Write(0xFF);
    receivedInterrupt_Enable();
    receivedInterrupt_StartEx(onUltrasonicReceived);
    //ledReg_Write(0xFF);
    ultrasonicTrigger_Write(0xFF);
    
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
