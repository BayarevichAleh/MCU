/*
 * IncFile1.h
 *
 * Created: 17.10.2019 10:40:25
 *  Author: bayar
 */ 




#ifndef LCD128X64_H_
	#define LCD128X64_H_
#endif

#ifndef SYMBOL_H_
	#include "symbol.h"
#endif /* SYMBOL_H_ */

#ifndef ON
	#define ON 1
#endif 
#ifndef OFF
	#define OFF 0
#endif

#define LCD_DATA PORTA
#define LCD_E PORTC5
#define LCD_DI PORTC6
#define LCD_RST PORTC2
#define LCD_CS1 PORTC4
#define LCD_CS2 PORTC3

void lcd_reset(unsigned char set){
	if(set)PORTC|=(1<<LCD_RST);
	else PORTC&=~(1<<LCD_RST);
}
void lcd_e(unsigned char set){
	if(set)PORTC|=(1<<LCD_E);
	else PORTC&=~(1<<LCD_E);
}
void lcd_di(unsigned char set){
	if(set)PORTC|=(1<<LCD_DI);
	else PORTC&=~(1<<LCD_DI);
}
void lcd_cs1(unsigned char set){
	if(set)PORTC|=(1<<LCD_CS1);
	else PORTC&=~(1<<LCD_CS1);
}
void lcd_cs2(unsigned char set){
	if(set)PORTC|=(1<<LCD_CS2);
	else PORTC&=~(1<<LCD_CS2);
}

//включение подсветки
void lcd_LED(unsigned char set){
	if(set){
		PORTC|=(1<<PORTC1);
		LCD_led_timer=240;
		led_use=0;
	}
	else PORTC&=~(1<<PORTC1);
}

void WriteCom(unsigned char COM,unsigned char CS){
	LCD_DATA=COM;
	lcd_di(OFF);
	lcd_cs1(CS%2);
	lcd_cs2(CS/2);
	_delay_loop_1(2);
	lcd_e(ON);
	_delay_loop_1(7);
	lcd_e(OFF);
}
void WriteData(unsigned char DATA,unsigned char CS){
	LCD_DATA=DATA;
	lcd_di(ON);
	lcd_cs1(CS%2);
	lcd_cs2(CS/2);
	_delay_loop_1(2);
	lcd_e(ON);
	_delay_loop_1(7);
	lcd_e(OFF);
}
void WriteXY(unsigned char x,unsigned char y,unsigned char CS){
	WriteCom(0b10111000+y,CS);
	WriteCom(0b01000000+x,CS);
}
void lcd_init(void){
	lcd_reset(ON);
	WriteCom(0b00111111,3); //lcd on
	WriteCom(0b11000000,3);	//scroll 0
	WriteXY(0,0,3);
}
void lcd_clean(void){
	unsigned char x,y;
	
	for(y=0;y<8;y++){
		for(x=0;x<64;x++){
			WriteXY(x,y,3);
			WriteData(0,3);
		}
	}
}
void lcd_set_ch(unsigned char CH, unsigned char x, unsigned char y){
	unsigned char CS=0;
	if(x<64){
		WriteXY(x,y,1);
		CS=1;
	}
	else {
		WriteXY(x-64,y,2);
		CS=2;
	}
	
	for(unsigned char i=0;i<5;i++){
		WriteData(pgm_read_byte(&(sym[CH][i])),CS);
		x++;
		if(x==64){
			x=0;
			CS=2;
			WriteXY(0,y,2);
		}
	}
	WriteData(0,CS);
}

void lcd_set_string(const char *s,unsigned char x, unsigned char y){
	//unsigned char i=0;
	while(*s){
		lcd_set_ch(*s++, x, y);
		x=x+6;
	}
}

//Вывод большой цифры
void lcd_set_big_digit(unsigned char DIGIT, unsigned char NUM){
	unsigned char CS=0,x=0;
	if(NUM==1){
		x=47;
		CS=1;
	}
	else {
		x=0;
		CS=2;
	}
	for(uint8_t j=0;j<3;j++){
		WriteXY(x,j+2,CS);
		for(unsigned char i=0;i<15;i++){
			WriteData(big_digit[DIGIT][i+(15*j)],CS);
		}
	}
}

