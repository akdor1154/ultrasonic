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
#include <stdio.h>

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
}

int _write(int file, char *ptr, int len) {

    kitProgUart_PutArray((uint8*)ptr, len);
    return len;
}

void inline triggerReading() {
    ultrasonicTrigger_Write(0xFF);
}

    

#define MAX_READINGS_TO_ATTEMPT 200
#define MAX_SUCCESSFUL_READINGS 10
#define READING_WAIT_TIME_MS 8
#define MIN_VALID_READING 5

uint16_t getAverageReading(int minSuccessfulReadings) {
    uint8_t successfulReadings = 0;
    uint32_t sumOfReadings = 0;
    int i = 0;
    while (successfulReadings < MAX_SUCCESSFUL_READINGS) {
        if (successfulReadings >= minSuccessfulReadings && i > MAX_READINGS_TO_ATTEMPT) {
            break;
        }
        uint16_t reading = 0;
        receiveTimeTarget = &(reading);
        triggerReading();
        CyDelay(READING_WAIT_TIME_MS);
        if (reading > MIN_VALID_READING) {
            sumOfReadings += reading;
            successfulReadings++;
        } else {
            printf(".");
        }
        i++;
    }
    return sumOfReadings/successfulReadings;
}

typedef enum {
    FlashSlow = 0b00,
    FlashFast = 0b01,
    On        = 0b10,
    Off       = 0b11
} LedMode;

void setLedMode(LedMode mode) {
    ledSpeedControl_Write(mode);
}

#define TIME_TO_WAIT_FOR_BUTTON_MS 20000
#define WAIT_FOR_BUTTON_LOOP_DELAY_MS 200
#define MAX_WAIT_FOR_BUTTON_ITERATIONS (TIME_TO_WAIT_FOR_BUTTON_MS/WAIT_FOR_BUTTON_LOOP_DELAY_MS)

inline int waitForOnboardButton() {
    //reset button register
    onboardButtonRegister_Read();
    int buttonPressed = 0;
    for (int i = 0; i<MAX_WAIT_FOR_BUTTON_ITERATIONS; i++) {
        buttonPressed = onboardButtonRegister_Read();
        if (buttonPressed) {
            break;
        }
        CyDelay(WAIT_FOR_BUTTON_LOOP_DELAY_MS);
    }
    return buttonPressed;
}

typedef struct Calibration {
    unsigned int a;
    signed int c;
} Calibration;

Calibration currentCalibration;
int calibrated = 0;

Calibration calculateCalibration(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
    unsigned int a = (y2 - y1) / (x2 - x1);
    signed int c = y2 - (a * x2);
    Calibration cal = {.a = a, .c = c};
    return cal;
}

unsigned int usToMm(unsigned int us, Calibration cal) {
    return (cal.a * us + cal.c) / 1000;
}

#define CALIBRATION_DISTANCE_1_UM 50000
#define CALIBRATION_DISTANCE_2_UM 150000

void calibrate() {
    
    unsigned int y1_um = CALIBRATION_DISTANCE_1_UM;
    unsigned int y2_um = CALIBRATION_DISTANCE_2_UM;
    
    // start light flashing
    setLedMode(FlashSlow);
    printf("Calibrate at %umm. \r\n", y1_um/1000);
    
    //wait for button
    waitForOnboardButton();
    
    //get reading for 100mm
    uint16_t x1_us = getAverageReading(5);
    printf("Time for %umm: %u microseconds. \r\n", y1_um/1000, x1_us);
    
    //flash faster
    setLedMode(FlashFast);
    printf("Now calibrate at %umm. \r\n", y2_um/1000);
    
    //wait for button
    waitForOnboardButton();
    
    //get reading for 400mm
    uint16_t x2_us = getAverageReading(5);
    printf("Time for %umm: %u microseconds. \r\n", y2_um/1000, x2_us);
    
    if (x1_us == 0 || x2_us == 0) {
        printf("Calibration failed! :( \r\n");
        return;
    }
    
    calibrated = 1;
    
    //calculate
    currentCalibration = calculateCalibration(x1_us, y1_um, x2_us, y2_um);
    printf("calibration results: um = %u * us  +  %i", currentCalibration.a, currentCalibration.c);
    
    printf(
        "Calibration check. %umm reading seems to be taken at %u. \r\n",
        y1_um, usToMm(x1_us, currentCalibration)
    );
    printf(
        "Calibration check. %umm reading seems to be taken at %u. \r\n",
        y2_um, usToMm(x2_us, currentCalibration)
    );
    //save results
    setLedMode(On);
    
    //print result
}

uint32_t calculateCalibrationChecksum(Calibration calibration) {
    uint32_t *calibrationAddr = (uint32_t*) &calibration;
    return 0xCCCCCCCC ^ calibrationAddr[0] ^ calibrationAddr[1];
}

int loadCalibration() {
    uint8_t* eepromBaseAddr = (uint8_t*) CYDEV_EE_BASE;
    uint32_t* calibrationChecksumAddr = (uint32_t*) eepromBaseAddr;
    Calibration* calibrationAddr = (Calibration*) (eepromBaseAddr + 4);
    
    uint32_t calibrationChecksum = *calibrationChecksumAddr;
    Calibration calibration = *calibrationAddr;
    
    if (calibrationChecksum != calculateCalibrationChecksum(calibration)) {
        printf("EEPROM appears to lack saved calibration Check: 0x%08x.\r\n", calibrationChecksum);
        return 0;
    }
    
    printf("Calibration in EEPROM seems valid. Loading... \r\n");
    calibrated = 1;
    currentCalibration = calibration;
    
    printf("Calibration loaded: um = %u * us  +  %u. \r\n", currentCalibration.a, currentCalibration.c);
    
    return 1;
}


void saveCalibration(Calibration cal) {
    size_t calibrationSize = sizeof(Calibration);
    uint8_t rowToWrite[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t* checkAddr = (uint32_t*) rowToWrite;
    Calibration* calibrationAddr = (Calibration*) (rowToWrite + 4);
    
    *checkAddr = calculateCalibrationChecksum(cal);
    *calibrationAddr = cal;
    
    printf("%08x %08x %08x %08x \r\n", *checkAddr, *(checkAddr+1), *(checkAddr+2), *(checkAddr+3));
    
    printf("Saving calibration to EEPROM. \r\n");
    
    cystatus writeStatus = eeprom_Write(rowToWrite, 0);
    if (writeStatus != CYRET_SUCCESS) {
        printf("write error: %li \r\n", writeStatus);
    } else {
        printf("saved. \r\n");
    }
}

uint8_t useInches;

CY_ISR_PROTO(onUnitSwitched);
CY_ISR(onUnitSwitched) {
    useInches = !(useInches);
}

typedef struct BCDNumber {
    uint8_t digit0;
    uint8_t digit1;
} BCDNumber;

BCDNumber intToBCDNumber(unsigned int num) {
    uint8_t ones = num % 10;
    uint8_t tens = num/10;
    BCDNumber bcdNumber = {.digit0 = ones, .digit1 = tens};
    return bcdNumber;
}

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
    
    unitSwitchInterrupt_Enable();
    unitSwitchInterrupt_StartEx(onUnitSwitched);
    //ledReg_Write(0xFF);
    ultrasonicTrigger_Write(0xFF);
    
    VDAC8_1_Start();
    eeprom_Start();
    CyDelay(5000);
    CySetTemp();
    if (!loadCalibration()) {
        calibrate();
        saveCalibration(currentCalibration);
    }
    
    const char mmStr[3] = "mm";
    const char inchStr[7] = "inches";
    
    while (1) {
        waitForOnboardButton();
        uint16_t averageReading = getAverageReading(0);
        if (averageReading == 0) {
            printf("failed.\r\n");
        } else if (calibrated) {
            uint16_t readingValue = usToMm(averageReading, currentCalibration);
            const char* unit;
            if (useInches) {
                readingValue = (readingValue * 10) / 254;
                unit = (char*) inchStr;
            } else {
                unit = (char*) mmStr;
            }
            printf("Reading guestimated at %u %s. \r\n", readingValue, unit);
        } else {
            printf("Reading took %u us, but we aren't calibrated. \r\n", averageReading);
        }
    }
}

/* [] END OF FILE */
