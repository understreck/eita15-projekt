#ifndef RFID_READER_H
#define RFID_READER_H

#include "spi.h"
#include "mfrc522.h"

#define UUID_LENGTH MAX_LEN

#include <stdbool.h>

struct RFID_UUID {
	uint8_t data[UUID_LENGTH];
};

bool
rfid_uuid_equal(struct RFID_UUID const* lhs, struct RFID_UUID const* rhs) {
	for(int i = 0; i < UUID_LENGTH; i++) {
		if(lhs->data[i] != rhs->data[i]) return false;
	}
	
	return true;
}

bool
rfid_read_card(struct RFID_UUID* out) {
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

#endif //RFID_READ_H