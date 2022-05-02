/*
 * anja-test.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"
#include "mfrc522.h"

int main(void)
{
    /* Replace with your application code */
    while (1) {
		spi_init();
		mfrc522_init();

		DDRB |= 0x06;
		uint8_t str[MAX_LEN];
		uint8_t msg[9];
		volatile uint16_t crc;
		uint32_t length;

		volatile uint8_t response = mfrc522_read(ComIEnReg);
		mfrc522_write(ComIEnReg, response|0x20);
		response = mfrc522_read(DivIEnReg);
		mfrc522_write(DivIEnReg, response|0x80);

		while(1){
			response = mfrc522_request(PICC_REQALL, str);

			if(response == CARD_FOUND) {

				response = mfrc522_get_card_serial(str);
				if(response == CARD_FOUND) {
					msg[0] = PICC_SElECTTAG;
					msg[1] = 0x70;
					for(uint8_t i = 0; i < 5; i++) {
						msg[2 + i] = str[i];
					}

					crc = mfrc522_calc_CRC(msg, 8);
					msg[7] = (uint8_t)crc;
					msg[8] = (uint8_t)(crc >> 8);

					response = mfrc522_to_card(Transceive_CMD, msg, 9, str, &length);
				}
			}
			else {
				PORTB &= ~0x06;
			}

			_delay_ms(500);
		}
    }
}

