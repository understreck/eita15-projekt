/*
 * anja-test.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000
#define GREEN 0x04
#define RED 0x02
#define BUZZ 0x01;

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#include "spi.h"
#include "mfrc522.h"

struct UID {
	uint8_t data[5];
};

bool
get_card(UID* out) {
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

int main(void)
{
    /* Replace with your application code */
    while (1) {
		spi_init();
		mfrc522_init();

		DDRB |= 0x07; //Buzzer and leds

		while(1) {
			
		}
    }
}

