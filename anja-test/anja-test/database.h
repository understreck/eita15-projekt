/*
 * database.h
 *
 * Created: 2022-05-17 16:37:46
 *  Author: le4112to-s
 */ 

#ifndef DATABASE_H
#define DATABASE_H

#include "rfid-reader.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>

#define EEPROM_END 4096
#define DB_MAX_ENTRIES 25
#define DB_MEM_POS 0

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

struct DB_Entry {  //Key value pair
	struct RFID_UUID uuid; //card ID
	char pwd[4]; //password with 4 characters
};

struct Database {
	uint8_t entries;
	struct DB_Entry kvps[DB_MAX_ENTRIES];
};

void
db_load(struct Database* database) {
	readFromEprom(DB_MEM_POS, database, sizeof(struct Database));
};

void
db_store(struct Database const* database) {
	writeToEprom(DB_MEM_POS, database, sizeof(struct Database));
}

struct DB_Entry const*
db_search(struct Database const* db, struct RFID_UUID const* uuid) {
	for(int i = 0; i < db->entries && i < DB_MAX_ENTRIES; i++) {
		if(rfid_uuid_equal(&db->kvps[i].uuid, uuid)) {
			return &db->kvps[i];
		}
	}
	
	return NULL;
}

bool
db_add(struct Database* db, struct DB_Entry const* kvp) { //returns false if database is full
	if(db->entries >= DB_MAX_ENTRIES){ return false;}
	
	db->kvps[db->entries] = *kvp;
	db->entries += 1;
	
	return true;
}

void
db_remove(struct Database* db, struct DB_Entry* kvp) {
	db->kvps[kvp - db->kvps] = db->kvps[db->entries - 1];
	db->entries = db->entries - 1;
}

#endif //DATABASE_H