/*
 * anja-test.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#include "spi.h"
#include "mfrc522.h"

#define GREEN (1 << PORTB2)
#define RED (1 << PORTB1)
#define BUZZ (1 << PORTB0)

#define LCD_CHAR_SELECT (1 << PORTD5)
#define LCD_RW_SELECT	(1 << PORTD6)
#define LCD_ENABLE		(1 << PORTD7)

struct UID {
	uint8_t data[MAX_LEN];
};

bool
get_card(struct UID* out) {
	uint8_t response = mfrc522_request(PICC_REQALL, out->data);
	
	if(response == CARD_FOUND) {
		response = mfrc522_get_card_serial(out->data);
		if(response == CARD_FOUND) {
			uint8_t checksum = out->data[0] ^ out->data[1] ^ out->data[2] ^ out->data[3];
					
			if(checksum == out->data[4]) {
				return true;
			}
		}
	}
	
	return false;
}

void
set_dbus(uint8_t b) {
	PORTC = (b & 0x03) | ((b << 4) & 0xC0);
	PORTD &= 0xE0;
	PORTD |= b >> 3;
}

void
lcd_write(char const* text, uint8_t length) {
	for(uint8_t i = 0; i < length; i++) {
		PORTD = LCD_CHAR_SELECT | LCD_ENABLE;
		set_dbus(text[i]);
		_delay_ms(1);
		PORTD &= ~LCD_ENABLE;
		_delay_ms(1);
	}
}

enum LCD_COMMAND {
	LCD_INIT = 0x0E,
	LCD_CLEAR = 0x01,
	LCD_HOME = 0x00
};

void
lcd_command(enum LCD_COMMAND c) {
	PORTD = LCD_ENABLE;
	set_dbus(c);
	_delay_ms(1);
	PORTD &= ~LCD_ENABLE;
	_delay_ms(1);
}

void
lcd_init() {
	DDRD |= 0xFF; //Display
	DDRC |= 0xFF; //Display
	
	lcd_command(LCD_INIT);
	lcd_command(LCD_CLEAR);
}
uint8_t global;
int main(void)
{
    /* Replace with your application code */
    while (1) {
		
    }
}