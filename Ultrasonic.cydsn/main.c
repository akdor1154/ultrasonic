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
#include <stdint.h>

int ultrasonicReceivedFlag = 0;
uint16_t receiveTimeMicroSeconds = 0;

uint16_t inline combineHighAndLow(uint8_t high, uint8_t low) {
    uint16_t returnInt = ((uint16_t) high << 8) | low;
    return returnInt;
}

uint16_t inline getReceiveTime() {
    uint8_t receiveTimeHigh = timeMicroSecondsHighReg_Read();
    uint8_t receiveTimeLow = timeMicroSecondsLowReg_Read();
    return combineHighAndLow(receiveTimeHigh, receiveTimeLow);
}

CY_ISR_PROTO(onUltrasonicReceived);
CY_ISR(onUltrasonicReceived) {
    ultrasonicReceivedFlag++;
    receiveTimeMicroSeconds = getReceiveTime();
    ledReg_Write(0xFF);
}

int _write(int file, char *ptr, int len) {

    kitProgUart_PutArray((uint8*)ptr, len);
    return len;
}


#include <stdio.h>

int main() {
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    ultrasonicRXThreshold_Start();
    ultrasonicRXComparator_Start();
    
    kitProgUart_Start();
    printf("Hello from main!\r\n");
    
    ultrasonicReset_Write(0xFF);
    receivedInterrupt_Enable();
    receivedInterrupt_StartEx(onUltrasonicReceived);
    //ledReg_Write(0xFF);
    ultrasonicTrigger_Write(0xFF);
    
    while (1) {
        CyDelay(1);
        if (ultrasonicReceivedFlag) {
            //printf("ISR has been called! There were %i flags. \r\n", ultrasonicReceivedFlag);
            //printf("Return time was %u. \r\n", receiveTimeMicroSeconds);
            ultrasonicReceivedFlag = 0;
            ledReg_Write(0xFF);
        }
        printf("Hello from the loop!\r\n");
        ultrasonicTrigger_Write(0xFF);
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
