#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/gpio/gpio.h"
#include "drivers/adc/adc.h"
#include "drivers/pwm/pwm.h"
#include "drivers/interrupt/external_interrupt.h"
#include "drivers/timer/timer0.h" 
#include "bsp/nano.h"
#include "utils/delay.h"

// Definiții pini
#define LED_SISTEM_OK    LED_BUILTIN 
#define LED_PWM_LUMINA   D11         
#define BUTON_URGENTA    D2          
#define CANAL_LDR        0           

// Stările sistemului
typedef enum {
    STATE_NORMAL,
    STATE_ALERTA
} SystemState_t;

volatile SystemState_t currentState = STATE_NORMAL;


void OnEmergencyPress(void) {
    currentState = STATE_ALERTA;
}

void Setup(void) {

    GPIO_Init(LED_SISTEM_OK, GPIO_OUTPUT);
    GPIO_Init(BUTON_URGENTA, GPIO_INPUT);
    GPIO_Write(BUTON_URGENTA, GPIO_HIGH); 

    ADC_Init();

    PWM_Init(LED_PWM_LUMINA, 1000); 

   
    ExtInt_Init(INT_0, EXT_INT_FALLING_EDGE, OnEmergencyPress);

    Timer0_Init(); 
    
    sei(); 
}

int main(void) {
    Setup();
    
    uint32_t lastCheck = 0;
    
    while (1) {
        if (currentState == STATE_NORMAL) {
            
            if (Millis() - lastCheck >= 200) {
                lastCheck = Millis();
                
              
                uint16_t lightVal = ADC_Read(CANAL_LDR);
                
                
                uint8_t duty = 255 - (lightVal / 4); 
                
                PWM_SetDutyCycle(LED_PWM_LUMINA, duty);
           
                GPIO_Toggle(LED_SISTEM_OK);
            }
        } 
        else if (currentState == STATE_ALERTA) {
            
            PWM_SetDutyCycle(LED_PWM_LUMINA, 0);
            GPIO_Write(LED_SISTEM_OK, GPIO_HIGH);
            Delay(100);
            GPIO_Write(LED_SISTEM_OK, GPIO_LOW);
            Delay(100);
            
            
            
        }
    }
}
