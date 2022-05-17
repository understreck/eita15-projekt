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

#define EEPROM_END 4096
#define DB_MAX_ENTRIES 100
#define DB_MEM_POS 0

//-------------------------------------RFID READER-----------------------------
struct UUID {
	uint8_t data[MAX_LEN];
};

bool
uuid_equal(struct UUID const* lhs, struct UUID const* rhs) {
	for(int i = 0; i < MAX_LEN; i++) {
		if(lhs->data[i] != rhs->data[i]) return false;
	}
	
	return true;
}

bool
get_card(struct UUID* out) {
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
	struct UUID uuid; //card ID
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
db_search(struct Database const* db, struct UUID const* uuid) {
	for(int i = 0; i < db->entries && i < DB_MAX_ENTRIES; i++) {
		if(uuid_equal(&db->kvps[i].uuid, uuid)) {
			return &db->kvps[i];
		}
	}
	
	return NULL;
}

/*
bool
db_add(struct Database* db, KVP const* kvp) { //returns false if database is full
	if(db->entries >= DB_MAX_ENTRIES) return false;
	
	//fortsätt
}
*/
/*
add

rotate (algoritm, kolla in array rotation)
remove
*/

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
	
	/*
	char const* string = "Hello World!"; //12 long
	writeToEprom((void*)2000, string, strlen(string));
	
	char stringCopy[12];
	readFromEprom((void*)2000, stringCopy, 12);
	
	lcd_write_n(stringCopy, 12);
	*/
}