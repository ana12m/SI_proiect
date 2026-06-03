/**
 * @file    main.c
 * @brief   Sistem de Control Ambiental Inteligent (Smart Home Node)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "drivers/gpio/gpio.h"
#include "drivers/adc/adc.h"
#include "drivers/pwm/pwm.h"
#include "drivers/interrupt/external_interrupt.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"

/* Pini */
#define LED_STATUS       LED_BUILTIN
#define LED_LUMINA       D11
#define LED_VENTILATOR   D9
#define BUTON_URGENTA    D2

#define CANAL_LDR        1
#define CANAL_TEMP       0

/* Praguri */
#define TEMP_PRAG_PORNIRE   400
#define TEMP_PRAG_MAXIM     900
#define INTERVAL_MS         200
#define INTERVAL_BLINK_MS   100

/* Stari */
typedef enum {
    STATE_NORMAL,
    STATE_ALERTA
} SystemState_t;

volatile SystemState_t currentState = STATE_NORMAL;

/* Callback intrerupere */
void OnEmergencyPress(void) {
    currentState = STATE_ALERTA;
}

/* Calcul duty ventilator */
static uint8_t CalcFanDuty(uint16_t tempVal) {
    if (tempVal <= TEMP_PRAG_PORNIRE) return 0;
    if (tempVal >= TEMP_PRAG_MAXIM)  return 255;
    uint32_t range  = TEMP_PRAG_MAXIM - TEMP_PRAG_PORNIRE;
    uint32_t offset = tempVal - TEMP_PRAG_PORNIRE;
    return (uint8_t)((offset * 255UL) / range);
}

/* Initializare */
void Setup(void) {
    GPIO_Init(LED_STATUS,     GPIO_OUTPUT);
    GPIO_Init(LED_LUMINA,     GPIO_OUTPUT);
    GPIO_Init(LED_VENTILATOR, GPIO_OUTPUT);
    GPIO_Init(BUTON_URGENTA,  GPIO_INPUT);
    GPIO_Write(BUTON_URGENTA, GPIO_HIGH); /* pull-up intern */

    ADC_Init();

    PWM_Init(LED_LUMINA,     1000);
    PWM_Init(LED_VENTILATOR, 1000);

    PWM_SetDutyCycle(LED_LUMINA,     0);
    PWM_SetDutyCycle(LED_VENTILATOR, 0);

    /* Intrerupere externa + polling ca backup */
    ExtInt_Init(INT_0, EXT_INT_FALLING_EDGE, OnEmergencyPress);

    Timer0_Init();
    sei();
}

/* Main */
int main(void) {
    Setup();

    uint32_t lastSensorCheck = 0;
    uint32_t lastBlink       = 0;
    uint8_t  butonPrecedent  = 1; /* 1 = neapasat (pull-up) */

    while (1) {

        /* Polling buton — detectare falling edge manual */
        uint8_t butonCurent = GPIO_Read(BUTON_URGENTA);
        if (butonPrecedent == 1 && butonCurent == 0) {
            currentState = STATE_ALERTA;
        }
        butonPrecedent = butonCurent;

        /* STATE NORMAL */
        if (currentState == STATE_NORMAL) {

            if (Millis() - lastSensorCheck >= INTERVAL_MS) {
                lastSensorCheck = Millis();

                uint16_t lightVal = ADC_Read(CANAL_LDR);
                uint16_t tempVal  = ADC_Read(CANAL_TEMP);

                uint8_t lightDuty = (uint8_t)(255U - (lightVal >> 2));
                PWM_SetDutyCycle(LED_LUMINA, lightDuty);

                uint8_t fanDuty = CalcFanDuty(tempVal);
                PWM_SetDutyCycle(LED_VENTILATOR, fanDuty);

                GPIO_Toggle(LED_STATUS);
            }
        }

        /* STATE ALERTA */
        else if (currentState == STATE_ALERTA) {

            PWM_SetDutyCycle(LED_LUMINA,     0);
            PWM_SetDutyCycle(LED_VENTILATOR, 0);

            if (Millis() - lastBlink >= INTERVAL_BLINK_MS) {
                lastBlink = Millis();
                GPIO_Toggle(LED_STATUS);
            }
        }
    }

    return 0;
}