#include <stdio.h>
#include "framework/test_framework.h"

// Include Mocks first
#include "mocks/avr/io.h"

// Include Driver to test
// We include the .c file directly to have access to everything, 
// and because we are not linking against the driver library in this simple setup
#include "../../drivers/gpio/gpio.c"

void test_gpio_init_output(void) {
    // Test initializing PB5 as OUTPUT
    GPIO_Init(GPIO_PORTB, 5, GPIO_OUTPUT);
    
    // Check if DDRB bit 5 is set
    ASSERT_EQUAL((1 << 5), DDRB & (1 << 5));
}

void test_gpio_init_input(void) {
    // Test initializing PB5 as INPUT
    // Set it to output first to ensure it changes
    DDRB = (1 << 5);
    GPIO_Init(GPIO_PORTB, 5, GPIO_INPUT);
    
    // Check if DDRB bit 5 is cleared
    ASSERT_EQUAL(0, DDRB & (1 << 5));
}

void test_gpio_write_high(void) {
    GPIO_Write(GPIO_PORTB, 5, GPIO_HIGH);
    ASSERT_EQUAL((1 << 5), PORTB & (1 << 5));
}

void test_gpio_write_low(void) {
    PORTB = (1 << 5); // Set high initially
    GPIO_Write(GPIO_PORTB, 5, GPIO_LOW);
    ASSERT_EQUAL(0, PORTB & (1 << 5));
}

void test_gpio_read(void) {
    PINB = (1 << 3); // Simulate Input High on Pin 3
    gpio_state_t state = GPIO_Read(GPIO_PORTB, 3);
    ASSERT_EQUAL(GPIO_HIGH, state);
    
    PINB = 0; // Simulate Input Low
    state = GPIO_Read(GPIO_PORTB, 3);
    ASSERT_EQUAL(GPIO_LOW, state);
}

int main(void) {
    printf("--- GPIO Driver Unit Tests ---\n");
    
    RUN_TEST(test_gpio_init_output);
    RUN_TEST(test_gpio_init_input);
    RUN_TEST(test_gpio_write_high);
    RUN_TEST(test_gpio_write_low);
    RUN_TEST(test_gpio_read);
    
    // Add more tests as needed
    
    return 0;
}
