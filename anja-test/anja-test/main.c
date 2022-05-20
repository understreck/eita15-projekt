/*
 * main.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000 //CPU clock speed (8MHz)

#include "keypad.h"
#include "database.h"
#include "lcd.h"
#include "rfid-reader.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <string.h>

#define GREEN	_BV(PORTB2)
#define RED		_BV(PORTB1)
#define BUZZ	_BV(PORTB0)

#define TIMER_VECTOR TIMER0_COMPA_vect

#define TIMER_INDEX_GREEN	0
#define TIMER_INDEX_RED		1
#define TIMER_INDEX_BUZZ	2
#define TIMER_INDEX_CARD	3
#define TIMER_INDEX_BUTTON	4

//Values are milliseconds
#define TIMER_GREEN			50
#define TIMER_RED			50
#define TIMER_BUZZ			50
#define TIMER_CARD			500
#define TIMER_LAST_INTERACT	4

#define TIMERS_LENGTH 5

uint16_t timers[TIMERS_LENGTH] = {0};

ISR(TIMER_VECTOR) {
	for(int i = 0; i < TIMERS_LENGTH; i++) {
		if(timers[i]) timers[i]--;
	}
}

void
timer_init() {
	TCCR0A	= _BV(WGM01); //CTC waveform generation mode
	TCCR0B	= _BV(CS02); //Divide clock signal by 256, 31.25kHz
	
	OCR0A	= 31; //Gives a clock of ~1kHz
	
	TIFR0	^= TIFR0; //Zero TC0 interrupt flag register
	TIMSK0	= _BV(OCIE0A); //Enable interrupt on OCR0A compare match
}

void
init() {
	_delay_ms(50);
	lcd_init();
	spi_init();
	mfrc522_init();
	kp_init();
	
	//Set leds and buzzer pins to outputs and then zero them
	DDRB |= GREEN | RED | BUZZ;
	PORTB &= ~(GREEN | RED | BUZZ);
	
	timer_init();
	sei(); //enable interrupts
}

int main(void)
{	
	init();
	
	while(1) {}
	
}
