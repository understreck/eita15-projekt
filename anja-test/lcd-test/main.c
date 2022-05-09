#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>

#define LCD_DATA PORTC                //LCD data port
#define ctrl PORTD
#define en PORTD7                      // enable signal
#define rw PORTD6                       // read/write signal
#define rs PORTD5                     // register select signal

void LCD_cmd(unsigned char cmd);

void init_LCD(void)
{
	LCD_cmd(0x38);                            // initialization of 16X2 LCD in 8bit mode
	_delay_ms(1);
	LCD_cmd(0x01);                                 // clear LCD
	_delay_ms(1);
	LCD_cmd(0x0E);                        // cursor ON
	_delay_ms(1);
	LCD_cmd(0x80);                     // �8 go to first line and �0 is for 0th position
	_delay_ms(1);
	return;
}

void LCD_cmd(unsigned char cmd)
{
	LCD_DATA=cmd;
	ctrl =(0<<rs)|(0<<rw)|(1<<en);
	_delay_ms(1);
	ctrl =(0<<rs)|(0<<rw)|(0<<en);
	_delay_ms(50);
	return;
}

void LCD_write(unsigned char data)
{
	LCD_DATA= data;
	ctrl = (1<<rs)|(0<<rw)|(1<<en);
	_delay_ms(1);
	ctrl = (1<<rs)|(0<<rw)|(0<<en);
	_delay_ms(50);
	return ;
}

void LCD_write_string(unsigned char *str)             //store address value of the string in pointer *str
{
	int i=0;
	while(str[i]!='\0')                               // loop will go on till the NULL character in the string
	{
		LCD_write(str[i]);                            // sending data on LCD byte by byte
		i++;
	}
	return;
}

int main()
{
	DDRC=0xff;                                  // setting the port B
	DDRD=0xE0;                                // setting for port D
	init_LCD();                                 // initialization of LCD
	_delay_ms(50);                        // delay of 50 mili seconds
	LCD_write_string("hello world");                      // function to print string on LCD
	return 0;
}

