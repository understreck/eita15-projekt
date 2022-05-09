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
#include <string.h>

#include "spi.h"
#include "mfrc522.h"

#define GREEN (1 << PORTB2)
#define RED (1 << PORTB1)
#define BUZZ (1 << PORTB0)

#define LCD_CHAR_SELECT (1 << PORTD5)
#define LCD_RW_SELECT	(1 << PORTD6)
#define LCD_ENABLE		(1 << PORTD7)

#define LCD_WIDTH 16

//-------------------------------------RFID READER-----------------------------
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
//-------------------------------------RFID READER-----------------------------

//-------------------------------------LCD SCREEN------------------------------
void
set_dbus(uint8_t b) {
	PORTC = (b & 0x03) | ((b << 4) & 0xC0);
	PORTD &= 0xE0;
	PORTD |= b >> 3;
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
	PORTD = LCD_ENABLE;
	set_dbus(c);
	_delay_us(150);
	PORTD &= ~LCD_ENABLE;
	_delay_us(1550);
}

void
lcd_init() {
	DDRD |= 0xFF; //Display
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
	
	PORTD = LCD_CHAR_SELECT | LCD_ENABLE;
	set_dbus(c);
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
hex_nibble_to_char(uint8_t a) {
	return a > 9 ? (a - 10) + 'A' : a + '0';
}

void
lcd_write_hex(uint8_t a) {
	lcd_write("0x");
	lcd_write_c(hex_nibble_to_char(a >> 4));
	lcd_write_c(hex_nibble_to_char(a & 0x0F));
}
//-------------------------------------LCD SCREEN------------------------------

void
init() {
	_delay_ms(50);
	lcd_init();
	spi_init();
	mfrc522_init();
}


int main(void)
{	
	init();
	
	struct UID uuid;
		
    while (1) {
		bool cardFound = get_card(&uuid);
		
		if(cardFound) {
			lcd_clear();
			
			lcd_write_hex(uuid.data[0]);
			lcd_write("  ");
			lcd_write_hex(uuid.data[1]);
			lcd_write_c('\n');
			
			
			lcd_write_hex(uuid.data[2]);
			lcd_write("  ");
			lcd_write_hex(uuid.data[3]);
		}
		_delay_ms(20);
    }
}