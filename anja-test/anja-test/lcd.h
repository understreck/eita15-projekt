/*
 * lcd.h
 *
 * Created: 2022-05-17 16:32:23
 *  Author: le4112to-s
 */ 

#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define LCD_CHAR_SELECT (1 << PORTD5)
#define LCD_RW_SELECT	(1 << PORTD6)
#define LCD_ENABLE		(1 << PORTD7)

#define LCD_WIDTH 16

void
lcd_set_dbus(uint8_t b) {
	PORTC = (b & 0x03) | ((b << 4) & 0xC0);
	PORTD &= 0xE1;
	PORTD |= b >> 3 & 0xFE;
}

enum LCD_COMMAND {
	LCD_START = 0x0E,
	LCD_CLEAR = 0x01,
	LCD_HOME = 0x00,
	LCD_CURSOR_TOP = (1 << 7) | 0x00,
	LCD_CURSOR_BOTTOM = (1 << 7) | 0x40,
	LCD_TWO_LINES = 0x38
};

void
lcd_command(enum LCD_COMMAND c) {
	PORTD |= LCD_ENABLE;
	PORTD &= ~(LCD_RW_SELECT | LCD_CHAR_SELECT);
	lcd_set_dbus(c);
	_delay_us(150);
	PORTD &= ~LCD_ENABLE;
	_delay_us(1550);
}

void
lcd_init() {
	DDRD |= 0xFE; //Display
	DDRC |= 0xFF; //Display
	
	lcd_command(LCD_START);
	lcd_command(LCD_CLEAR);
	lcd_command(LCD_TWO_LINES);
}

void
lcd_clear() {
	lcd_command(LCD_CLEAR);
}

void
lcd_write_c(char c) {
	if(c == '\n') {
		lcd_command(LCD_CURSOR_BOTTOM);
		return;
	}
	
	PORTD |= LCD_CHAR_SELECT | LCD_ENABLE;
	PORTD &= ~LCD_RW_SELECT;
	lcd_set_dbus(c);
	_delay_us(150);
	PORTD &= ~LCD_ENABLE;
	_delay_us(150);
}

void
lcd_write_n(char const* text, uint8_t length) {
	
	
	for(uint8_t i = 0; i < length; i++) {
		lcd_write_c(text[i]);
	}
}

void
lcd_write(char const* text) {
	lcd_write_n(text, strlen(text));
}

char
lcd_hex_nibble_to_char(uint8_t a) {
	return a > 9 ? (a - 10) + 'A' : a + '0';
}

void
lcd_write_hex(uint8_t a) {
	lcd_write("0x");
	lcd_write_c(lcd_hex_nibble_to_char(a >> 4));
	lcd_write_c(lcd_hex_nibble_to_char(a & 0x0F));
}

#endif //LCD_H