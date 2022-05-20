/*
 * main.c
 *
 * Created: 2022-04-28 22:07:51
 * Author : ETF
 */ 

#define F_CPU 8000000 //CPU clock speed (8MHz)

#include "keypad.h"
#include "database.h"
#include "lcd.h"
#include "rfid-reader.h"
#include "led_and_buzz.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdbool.h>
#include <string.h>

#define CODE_BUFF_SIZE 4

#define TIMER_VECTOR TIMER0_COMPA_vect

#define TIMER_INDEX_GREEN	0
#define TIMER_INDEX_RED		1
#define TIMER_INDEX_BUZZ	2
#define TIMER_INDEX_CARD	3
#define TIMER_INDEX_TIMEOUT	4

//Values are milliseconds
#define TIMER_GREEN			200
#define TIMER_RED			200
#define TIMER_BUZZ			50
#define TIMER_CARD			2000
#define TIMER_TIMEOUT		15000

#define TIMERS_LENGTH 5

uint16_t timers[TIMERS_LENGTH] = {0};
	
ISR(TIMER_VECTOR) {
	for(int i = 0; i < TIMERS_LENGTH; i++) {
		if(timers[i]) {
			if(--timers[i] == 0) {
				switch(i) {
				case TIMER_INDEX_GREEN: {
					led_green_off();
				} break;
				case TIMER_INDEX_RED: {
					led_red_off();
				} break;
				case TIMER_INDEX_BUZZ: {
					buzz_off();
				} break;
				}
			}
		}
	}
}

void
timer_init() {
	TCCR0A	= _BV(WGM01); //CTC waveform generation mode
	TCCR0B	= _BV(CS02); //Divide clock signal by 256, 31.25kHz
	
	OCR0A	= 31; //Gives a clock of ~1kHz
	
	TIFR0	^= TIFR0; //Zero TC0 interrupt flag register
	TIMSK0	= _BV(OCIE0A); //Enable interrupt on OCR0A compare match
}

void
init() {
	_delay_ms(50);
	lcd_init();
	spi_init();
	mfrc522_init();
	kp_init();
	timer_init();
	led_and_buzz_init();
	
	sei(); //enable interrupts
}

enum State {
	BASE,
	RESET,
	AUTH,
	OPCODE_ENTRY,
	SEARCH,
	FAILURE,
	SUCCESS,
	REMOVING,
	REMOVE,
	ADDING,
	ADD
};

char const OPCODE_add[CODE_BUFF_SIZE]		= {'1','1','1','1'};
char const OPCODE_remove[CODE_BUFF_SIZE]	= {'2','2','2','2'};
char const OPCODE_clear[CODE_BUFF_SIZE]		= {'3','3','3','3'};
char const OPCODE_count[CODE_BUFF_SIZE]		= {'4','4','4','4'};
	
char const* const OPCODES[4] = {
	OPCODE_add, OPCODE_remove, OPCODE_clear, OPCODE_count
};

enum OPCODE {
	OP_ADD,
	OP_REMOVE,
	OP_CLEAR,
	OP_COUNT,
	OP_NONE
};

enum OPCODE
get_opcode(char const* code) {
	for(int i = 0; i < 4; i++) {
		bool match = true;
		for(int j = 0; j < CODE_BUFF_SIZE; j++) {
			if(OPCODES[i][j] != code[j]) {
				match = false;
				break;
			}
		}
		if(match) {
			switch(i) {
				case 0: return OP_ADD;
				case 1: return OP_REMOVE;
				case 2: return OP_CLEAR;
				case 3: return OP_COUNT;
			}
		}
	}
	
	return OP_NONE;
};

void
signal_success() {
	buzz_on();
	timers[TIMER_INDEX_BUZZ] = TIMER_BUZZ;
	
	led_green_on();
	timers[TIMER_INDEX_GREEN] = TIMER_GREEN;
}

void
signal_failure() {
	buzz_on();
	timers[TIMER_INDEX_BUZZ] = TIMER_BUZZ * 3;
	
	led_red_on();
	timers[TIMER_INDEX_RED] = TIMER_RED;
}

int main(void)
{	
	init();
	struct Database db;
	db_load(&db);

	bool timeOutActive = false;

	char codeBuff[CODE_BUFF_SIZE] = {0};
	int nrEntered = 0;

	enum State oldState = RESET;
	struct DB_Entry const* activeEntry = NULL;
	struct DB_Entry newEntry = {0};
	char oldChar = '\0';

	bool awaitingCard = false;
	
	while(1) {
		enum State newState = oldState;
		
		if(timeOutActive && timers[TIMER_INDEX_TIMEOUT] == 0) {
			newState = RESET;
			timeOutActive = false;
		}
		
		struct RFID_UUID newUUID;
		if(timers[TIMER_INDEX_CARD] == 0) {
			if(rfid_read_card(&newUUID)) {
				timers[TIMER_INDEX_CARD] = TIMER_CARD;
				
				if(oldState == REMOVE && awaitingCard) {
					timers[TIMER_INDEX_TIMEOUT] = TIMER_TIMEOUT;
					nrEntered = 0;
					awaitingCard = false;
					
					if(db_search(&db, &newUUID) == NULL) {
						lcd_clear();
						lcd_write("Unknown card");
						newState = FAILURE;
					}
				}
				else if(oldState == ADD && awaitingCard) {
					timers[TIMER_INDEX_TIMEOUT] = TIMER_TIMEOUT;
					nrEntered = 0;
					awaitingCard = false;
					
					if(db_search(&db, &newUUID) != NULL) {
						lcd_clear();
						lcd_write("Card already in\n database");
						newState = FAILURE;
					}
					else {
						lcd_clear();
						lcd_write("Enter pin code: \n");
						
						buzz_on();
						timers[TIMER_INDEX_BUZZ] = TIMER_BUZZ;
						newEntry.uuid = newUUID;
					}
				}
				else {
					oldState = RESET;
					newState = SEARCH;
				}
			}
		}
		
		char newChar = kp_read_char();
		if(newChar != oldChar && newChar != '\0') {
			if(newState == BASE) {
				newState = OPCODE_ENTRY;
				lcd_clear();
			}
			
			if(nrEntered < CODE_BUFF_SIZE) {
				codeBuff[nrEntered++] = newChar;
			}
			
			lcd_write_c('*');
			buzz_on();
			timers[TIMER_INDEX_BUZZ] = TIMER_BUZZ / 2;
		}
			
		switch(newState) {
			case SEARCH: {
				lcd_clear();
				activeEntry = db_search(&db, &newUUID);
				if(activeEntry != NULL) {
					buzz_on();
					timers[TIMER_INDEX_BUZZ] = TIMER_BUZZ;
					
					led_green_on();
					timers[TIMER_INDEX_GREEN] = TIMER_GREEN;
					
					lcd_write("Enter pin code: \n");
					nrEntered = 0;
					newState = AUTH;
				}
				else {
					lcd_write("Unknown card");
					newState = FAILURE;
				}
			} break;
			case OPCODE_ENTRY: {
				if(nrEntered == CODE_BUFF_SIZE) {
					nrEntered = 0;
					switch(get_opcode(codeBuff)) {
						case OP_COUNT: {
							lcd_clear();
							lcd_write_c(db.entries + '0');
							
							newState = SUCCESS;
						} break;
						case OP_CLEAR: {
							lcd_clear();
							
							struct Database zeroedDB = {0};
							db = zeroedDB;
							db_store(&db);
							
							lcd_write("Database emptied");
							
							newState = SUCCESS;
						} break;
						case OP_ADD: {
							lcd_clear();
							if(db.entries >= DB_MAX_ENTRIES) {
								lcd_write("DB full");
								newState = FAILURE;
							}
							else {
								lcd_write("Scan card to add");
								awaitingCard = true;
								newState = ADD;
							}
						} break;
						case OP_REMOVE: {
							lcd_clear();
							lcd_write("Scan card to \nremove");
							awaitingCard = true;
							newState = REMOVE;
						} break;
						case OP_NONE:
						default: {
							lcd_clear();
							lcd_write("Unknown code");
							
							newState = FAILURE;
						}
					}
				}
			} break;
			case ADD: {
				if(!awaitingCard) {
					if(nrEntered == CODE_BUFF_SIZE) {
						for(int i = 0; i < CODE_BUFF_SIZE; i++) {
							newEntry.pwd[i] = codeBuff[i];
						}
						db_add(&db, &newEntry);
						db_store(&db);
						
						lcd_clear();
						lcd_write("Card added");
						
						newState = SUCCESS;
					}
				}
			} break;
			case REMOVE: {
				if(!awaitingCard) {
					lcd_clear();
					
					db_remove(&db, db_search(&db, &newUUID));
					db_store(&db);
					
					lcd_write("Removed card");
					
					newState = SUCCESS;
				}	
			} break;
			case AUTH: {
				if(nrEntered == CODE_BUFF_SIZE) {
					lcd_clear();
					
					bool match = true;
					for(int i = 0; i < CODE_BUFF_SIZE; i++) {
						if(activeEntry->pwd[i] != codeBuff[i]) {
							match = false;
							break;
						}
					}
					
					if(match) {
						lcd_write("Opening!");
						newState = SUCCESS;
					}
					else {
						lcd_write("Wrong pin code!");
						newState = FAILURE;
					}
				}
			} break;
			case RESET: {
				buzz_off();
				led_green_off();
				led_red_off();
				lcd_clear();
				awaitingCard = false;
				timeOutActive = false;
				activeEntry = NULL;
				nrEntered = 0;
					
				newState = BASE;
			} break;
			case SUCCESS: {
				signal_success();
				newState = BASE;
			} break;
			case FAILURE: {
				signal_failure();
				newState = BASE;
			} break;
			case BASE: {
				nrEntered = 0;
			} break;
		}
			
		if(	(oldState != newState && oldState != BASE) ||
			(newChar != oldChar && newChar != '\0')) {
			timers[TIMER_INDEX_TIMEOUT] = TIMER_TIMEOUT;
			timeOutActive = true;
		}
		oldChar = newChar;
		oldState = newState;
	}
}
