/*
 * led_and_buzz.h
 *
 * Created: 2022-05-20 17:28:56
 *  Author: le4112to-s
 */ 


#ifndef LED_AND_BUZZ_H
#define LED_AND_BUZZ_H

#include <avr/io.h>

#define GREEN	_BV(PORTD0)
#define RED		_BV(PORTB2)
#define BUZZ	_BV(PORTB0)

void
led_red_on() {
	PORTB |= RED;
}

void
led_red_off() {
	PORTB &= ~RED;
}

void
led_green_on() {
	PORTD |= GREEN;
}

void
led_green_off() {
	PORTD &= ~GREEN;
}

void
buzz_on() {
	PORTB |= BUZZ;
}

void
buzz_off() {
	PORTB &= ~BUZZ;
}

void
led_and_buzz_init() {
	//Set leds and buzzer pins to outputs and then zero them
	DDRB |= RED | BUZZ;
	DDRD |= GREEN;
	PORTB &= ~(RED | BUZZ);
	PORTD &= ~GREEN;
}

#endif //LED_AND_BUZZ_H