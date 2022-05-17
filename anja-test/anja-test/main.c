/*
 * anja-test.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000 //CPU clock speed (8MHz)

#include "lcd.h"
#include "rfid-reader.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

#define GREEN (1 << PORTB2)
#define RED (1 << PORTB1)
#define BUZZ (1 << PORTB0)

#define EEPROM_END 4096
#define DB_MAX_ENTRIES 100
#define DB_MEM_POS 0

//-------------------------------------DATABASE--------------------------------

void EEPROM_write(void* uiAddress, char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = (uint16_t)uiAddress;
	EEDR = ucData;

	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);

	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

char EEPROM_read(void const* uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = (uint16_t)uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

void writeToEprom (void* eepromAdress, void const* data, size_t length){
	for(size_t i = 0; i < length; i++) {
		EEPROM_write(eepromAdress + i, ((char*)data)[i]);
	}
}

void readFromEprom (void const* eepromAdress, void* data, size_t length){
	for(size_t i = 0; i < length; i++) {
		((char*)data)[i] = EEPROM_read(eepromAdress + i);
	}
}

struct KVP {  //Key value pair
	struct RFID_UUID uuid; //card ID
	char pwd[4]; //password with 4 characters
};

struct Database {
	uint8_t entries;
	struct KVP kvps[DB_MAX_ENTRIES];
};

void
db_load(struct Database* database) {
	readFromEprom(DB_MEM_POS, database, sizeof(struct Database));
};

void
db_store(struct Database const* database) {
	writeToEprom(DB_MEM_POS, database, sizeof(struct Database));
}

struct KVP const*
db_search(struct Database const* db, struct RFID_UUID const* uuid) {
	for(int i = 0; i < db->entries && i < DB_MAX_ENTRIES; i++) {
		if(rfid_uuid_equal(&db->kvps[i].uuid, uuid)) {
			return &db->kvps[i];
		}
	}
	
	return NULL;
}

bool
db_add(struct Database* db, struct KVP const* kvp) { //returns false if database is full
	if(db->entries >= DB_MAX_ENTRIES){ return false;}
	
	db->kvps[db->entries] = *kvp;
	db->entries += 1;
	
	return true;
}

void
db_remove(struct Database* db, struct KVP* kvp) {
	db->kvps[kvp - db->kvps] = db->kvps[db->entries - 1];
	db->entries = db->entries - 1;
}

//-------------------------------------DATABASE--------------------------------

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