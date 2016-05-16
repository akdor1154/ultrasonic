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
uint16_t* receiveTimeTarget = (uint16_t*) 0;

void inline writeToReadingTarget(uint16_t time) {
    if (receiveTimeTarget) {
        (*receiveTimeTarget) = time;
    }
    receiveTimeTarget = (uint16_t*) 0;
}

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
    writeToReadingTarget(receiveTimeMicroSeconds);
    ledReg_Write(0xFF);
}

int _write(int file, char *ptr, int len) {

    kitProgUart_PutArray((uint8*)ptr, len);
    return len;
}


void inline triggerReading() {
    ultrasonicTrigger_Write(0xFF);
}

    

#define READINGS_TO_TAKE 20
#define MS_BETWEEN_READINGS 5
#define MIN_VALID_READING 5

uint16_t getAverageReading() {
    uint16_t readings[READINGS_TO_TAKE];
    uint8_t successfulReadings = 0;
    uint32_t sumOfReadings = 0;
    for (int i = 0; i < READINGS_TO_TAKE; i++) {
        readings[i] = 0;
        receiveTimeTarget = &(readings[i]);
        triggerReading();
        CyDelay(MS_BETWEEN_READINGS);
        if (readings[i] > MIN_VALID_READING) {
            sumOfReadings += readings[i];
            successfulReadings++;
        }
    }
    return sumOfReadings/successfulReadings;
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
    
    VDAC8_1_Start();
    
    while (1) {
        CyDelay(1000);
        uint16_t averageReading = getAverageReading();
        printf("average reading: %u \r\n", averageReading);
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
