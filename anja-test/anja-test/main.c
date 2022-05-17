/*
 * anja-test.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000 //CPU clock speed (8MHz)

#include "database.h"
#include "lcd.h"
#include "rfid-reader.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

#define GREEN (1 << PORTB2)
#define RED (1 << PORTB1)
#define BUZZ (1 << PORTB0)

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
	
	struct Database db;
	db_load(&db);
}