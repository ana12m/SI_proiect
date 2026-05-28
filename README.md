# Sistem de Control Ambiental Inteligent 

> Proiect integrat pentru cursul de Sisteme Incorporate, Automatică și Informatică Aplicată,  
> Facultatea de Automatică, Calculatoare și Electronică, Universitatea din Craiova.



## Descrierea Proiectului

Acest proiect implementează un **Sistem de Control Ambiental Inteligent (Smart Home Node)** pe microcontrollerul (Arduino Nano/Uno). Sistemul monitorizează mediul în timp real prin doi senzori analogici și reacționează automat prin două canale PWM, fără a bloca procesorul. Un buton de urgență declanșează hardware o stare de alertă prin întrerupere externă.

Proiectul integrează toate conceptele din laboratoare într-o logică de control unitară:

| Laborator | Modul | Rol în sistem |
|-----------|-------|---------------|
| Lab 1 | GPIO | LED status sistem, configurare button cu pull-up |
| Lab 2 | Întreruperi Externe (INT0) | Buton urgență — declanșare alertă hardware |
| Lab 3 | Timer0 / `Millis()` | Bază de timp neblocantă — citire senzori la 200 ms |
| Lab 4 | PWM (Timer1, Timer2) | Control lumină ambientală și ventilator prin duty cycle |
| Lab 5 | ADC | Citire senzor lumină (A0) și senzor temperatură (A1) |



## Schema de Conectare


Arduino Nano/Uno
│
├── D13 (LED_BUILTIN) ── LED Status sistem
│                         Blink lent  (500ms) = Sistem OK
│                         Blink rapid (100ms) = ALERTĂ
│
├── D11 (PWM, Timer2) ── LED / Bec dimabil
│                         Intensitate invers proporțională cu lumina din cameră
│
├── D9  (PWM, Timer1) ── LED / Motor ventilator
│                         Viteză proporțională cu temperatura citită
│
├── D2  (INT0)        ── Buton urgență (cu pull-up intern)
│                         Apăsare → front descrescător → ISR → STATE_ALERTA
│
├── A0  (ADC Ch.0)    ── LDR / Potențiometru (simulare lumină ambientală)
│
└── A1  (ADC Ch.1)    ── Senzor NTC / Potențiometru (simulare temperatură)



## Logica Sistemului — Mașina de Stări


                    ┌─────────────────────┐
          Reset     │                     │
        ──────────► │    STATE_NORMAL     │
                    │                     │
                    │  • Citire LDR (A0)  │
                    │  • Citire Temp (A1) │   Apăsare buton D2
                    │  • PWM Lumina auto  │ ─────────────────────►
                    │  • PWM Ventil. auto │
                    │  • LED blink lent   │
                    └─────────────────────┘

                    ┌─────────────────────┐
                    │                     │
                    │    STATE_ALERTA     │
                    │                     │
                    │  • PWM Lumina = 0   │
                    │  • PWM Ventil. = 0  │
                    │  • LED blink rapid  │
                    │                     │
                    └─────────────────────┘
                    (Ieșire: reset hardware)


### Logica de control PWM

**Lumina ambientală (D11):**

lightDuty = 255 - (ADC_Read(A0) / 4)

ADC = 0    ( lumină maximă) → duty = 255 → LED la maxim
ADC = 512  (lumină medie)   → duty = 127 → LED la 50%
ADC = 1023 (întuneric total)  → duty = 0   → LED stins


## Structura Proiectului


├── bsp/                    # Definire pini pentru plăci (uno.h, nano.h)
├── drivers/                # Strat de abstractizare hardware (HAL)
│   ├── adc/                # Driver ADC — conversie 10-bit blocantă
│   ├── eeprom/             # Driver EEPROM — Read/Write/Update
│   ├── gpio/               # Driver GPIO — Init/Write/Read/Toggle
│   ├── interrupt/          # Driver întreruperi externe INT0/INT1
│   ├── pwm/                # Driver PWM — wrapper Timer1 și Timer2
│   └── timer/              # Driver Timer0 — Millis() la 1ms
├── src/
│   └── main.c              # Logica principală — mașina de stări
├── test/                   # Teste unitare pe host (fără hardware)
│   ├── framework/          # Runner minimal pentru teste
│   ├── mocks/              # Mock registre AVR pentru simulare
│   └── test_*.c            # Fișiere de test (GPIO, PWM)
├── utils/                  # Macro-uri utilitare (Delay, BIT ops)
└── Makefile                # Compilare, flash, teste, coverage


## Build & Flash

### Cerințe

- `avr-gcc` toolchain
- `avrdude`
- `make`
- `gcc` + `lcov` (opțional, pentru teste și coverage)

### Comenzi principale

| Comandă | Descriere |
|---------|-----------|
| `make all BOARD=nano` | Compilare pentru Arduino Nano |
| `make all BOARD=uno` | Compilare pentru Arduino Uno |
| `make flash` | Încărcare firmware pe placă (port COM3, 57600 baud) |
| `make clean` | Ștergere artefacte de build |

> **Port serial:** Modifică variabila `PORT` în `Makefile` dacă placa nu este pe `COM3`  
> (ex: `PORT = /dev/ttyUSB0` pe Linux, `PORT = /dev/cu.usbserial-*` pe macOS).


## Teste și Code Coverage

Proiectul suportă rularea testelor unitare direct pe calculatorul tău (fără hardware) prin simularea registrelor AVR.

### Comenzi

| Comandă | Descriere |
|---------|-----------|
| `make test` | Compilare și rulare toate testele (GPIO, PWM) |
| `make coverage` | Rulare teste + generare raport gcov |
| `make coverage-html` | Generare raport HTML vizual (necesită `lcov`) |



## Drivere disponibile

| Driver | Funcții principale | Status |
|--------|-------------------|--------|
| GPIO | `GPIO_Init`, `GPIO_Write`, `GPIO_Read`, `GPIO_Toggle` | ✅ |
| Interrupt | `ExtInt_Init` cu callback | ✅ |
| Timer0 | `Timer0_Init`, `Millis()` (CTC 1ms) | ✅ |
| ADC | `ADC_Init`, `ADC_Read` (10-bit blocant) | ✅ |
| PWM | `PWM_Init`, `PWM_SetDutyCycle` (Timer1 + Timer2) | ✅ |
| EEPROM | `EEPROM_Read`, `EEPROM_Write`, `EEPROM_Update` | ✅ |
| I2C | — | 🔲 Planificat |
| SPI | — | 🔲 Planificat |
| UART | — | 🔲 Planificat |

---

## Exemplu de utilizare a driverelor

```c
#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "drivers/adc/adc.h"
#include "drivers/pwm/pwm.h"
#include "bsp/nano.h"

int main(void) {
    Timer0_Init();
    ADC_Init();
    GPIO_Init(LED_BUILTIN, GPIO_OUTPUT);
    PWM_Init(D11, 1000);   /* 1 kHz pe D11 */

    sei();

    while (1) {
        uint16_t val = ADC_Read(0);            /* Citire A0             */
        PWM_SetDutyCycle(D11, 255 - (val>>2)); /* Inversare + scalare   */
        GPIO_Toggle(LED_BUILTIN);
        /* Delay neblocant: foloseste Millis() in productie */
    }
}
