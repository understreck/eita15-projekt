/*
 * keypad.h
 *
 * Created: 2022-05-17 16:58:27
 *  Author: le4112to-s
 */ 

#ifndef KEYPAD_H
#define KEYPAD_H

#include <avr/io.h>
#include <util/delay.h>

#define KP_NUM_ROWS 4
#define KP_NUM_COLS 4

#define KP_ROW_0 0b00000001
#define KP_ROW_1 0b00000010
#define KP_ROW_2 0b00000100
#define KP_ROW_3 0b00001000
#define KP_ROWS  0x0F

#define KP_COL_0 0b00010000
#define KP_COL_1 0b00100000
#define KP_COL_2 0b01000000
#define KP_COL_3 0b10000000
#define KP_COLS  0xF0

#ifndef KP_DEBOUNCE_DELAY
	#define KP_DEBOUNCE_DELAY 1
#endif //KP_DEBOUNCE_DELAY

enum KP_CODE {
    KP_1 = KP_ROW_0 | KP_COL_0,
    KP_2 = KP_ROW_0 | KP_COL_1,
    KP_3 = KP_ROW_0 | KP_COL_2,

    KP_4 = KP_ROW_1 | KP_COL_0,
    KP_5 = KP_ROW_1 | KP_COL_1,
    KP_6 = KP_ROW_1 | KP_COL_2,

    KP_7 = KP_ROW_2 | KP_COL_0,
    KP_8 = KP_ROW_2 | KP_COL_1,
    KP_9 = KP_ROW_2 | KP_COL_2,
	
	KP_0 = KP_ROW_3 | KP_COL_1,
	
	KP_SQUARE	= KP_ROW_3 | KP_COL_2,
	KP_STAR		= KP_ROW_3 | KP_COL_0,
	
	KP_A = KP_ROW_0 | KP_COL_3,
	KP_B = KP_ROW_1 | KP_COL_3,
	KP_C = KP_ROW_2 | KP_COL_3,
	KP_D = KP_ROW_3 | KP_COL_3,
	
	KP_NONE = '\0'
};

void
kp_init() {
	DDRA  = 0xFF; //4 high bits are columns and inputs, 4 low rows and outputs
	PORTA = 0x00; //Zero all outputs
	DDRA  = 0x0F;
}

enum KP_CODE
kp_read_row(uint8_t row) {
	PORTA = row;
	_delay_us(1);
	
	enum KP_CODE result = PINA;
	
	PORTA = 0x00;	
	return result;
}

enum KP_CODE
kp_read_code() {
	enum KP_CODE result;
	
	for(int i = 0; i < KP_NUM_ROWS; i++) {
		result = kp_read_row(1 << i);
		if(result & KP_COLS) { //If a column pin was high on the row
			return result;
		}
	}
	
	return KP_NONE;
}

enum KP_CODE
kp_read_code_debounce() {
	enum KP_CODE result = kp_read_code();
	_delay_ms(KP_DEBOUNCE_DELAY);
	
	if(result == kp_read_code()) {
		return result;
	}
	
	return KP_NONE;
}

char
kp_read_char() { //if no character is read, return null character
	switch(kp_read_code_debounce()) {
		case KP_1: return '1';
		case KP_2: return '2';
		case KP_3: return '3';
		
		case KP_4: return '4';
		case KP_5: return '5';
		case KP_6: return '6';
		
		case KP_7: return '7';
		case KP_8: return '8';
		case KP_9: return '9';
		
		case KP_STAR:	return '*';
		case KP_0:		return '0';
		case KP_SQUARE:	return '#';
		
		case KP_A: return 'A';
		case KP_B: return 'B';
		case KP_C: return 'C';
		case KP_D: return 'D';
		
		KP_NONE:
		default: {
			return '\0';
		}
	}
}

#endif //KEYPAD_H