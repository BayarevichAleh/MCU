/*
 * PU.c
 *
 * Created: 15.02.2020 18:26:17
 * Author : bayar
 */ 

#define F_CPU 16000000L
#define ON 1
#define OFF 0
#define DPNK 1
#define DK 0
#define RS485_ON PORTD|=(1<<PORTD2)
#define RS485_OFF PORTD&=~(1<<PORTD2)
#define START_BYTE 0x3A
#define STOP_BYTE 0xDA
#define BLOCK 0x42
#define OPEN 0x4F
#define DISCONNECT 0x44
#define ERROR 0x45

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <stdint.h>
//#include <avr/eeprom.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

volatile uint8_t timer=0;
volatile uint8_t sistem_mode=1;
volatile uint8_t PU_mode=0;
volatile uint8_t BUT_FIX=0;
volatile uint8_t BUTTON=15;
volatile uint8_t page=0;
volatile uint8_t update=0;
volatile uint8_t big_digit_1=10;
volatile uint8_t big_digit_2=10;
volatile uint8_t DPNK_BLOCK=BLOCK;
volatile uint8_t beep_time=0;
volatile uint8_t transmit=0;
volatile uint8_t UART_byte=0;
volatile uint8_t UART_adress=0;
volatile uint8_t bufer=0;
volatile uint8_t package[80];
volatile uint8_t RX_index=0;
volatile uint8_t CRC=0;
volatile uint8_t open_EMZ=OPEN;
volatile uint8_t DPNK_ERROR=0;
volatile uint8_t user_password[4]={0,0,0,0};
volatile uint8_t admin_password[4]={0,0,0,0};
volatile uint8_t bufer_password[4]={10,10,10,10};
volatile uint8_t reset_password=0;
volatile uint8_t timer_DPNK=0;
volatile uint8_t DPNK_wireles=0;
volatile uint8_t scan_on=0;
volatile uint8_t scan_index=0;
volatile uint8_t password_error=0;
volatile uint8_t CRC_IN=0;

struct EMZ
{
	uint8_t status[64];
	uint8_t out[64];
	uint8_t error[32];
}EMZ={
	{BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK},
	{BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};


#include "USART.h"
#include "lcd128x64.h"
#include "Functions.h"
#include "LCDmenu.h"


ISR(USART0_RX_vect){
	bufer=UDR0;
	if(bufer==START_BYTE){
		package[1]=0;
		package[2]=0;
		package[6]=0;
		package[7]=0;
		package[8]=0;
		package[73]=0;
		package[74]=0;
		RX_index=0;
	}
	if(RX_index<75){
		package[RX_index]=bufer;
		RX_index++;
	}
}

ISR(TIMER0_OVF_vect){
	if(beep_time){
		beep_time--;
		if(beep_time==10)PORTC&=~(1<<PORTC0);
	}
	else{
		TIMSK0=OFF;
	}
}

ISR (TIMER1_OVF_vect){
	if(timer_DPNK)timer_DPNK--;
	else {
		TIMSK1=OFF;
	}
}

ISR (TIMER2_OVF_vect){
	if(timer)timer--;
	else {
		TIMSK2=OFF;
	}
}


int main(void)
{
    DDRA=0b11111111;
	DDRB=0b00011100;
	DDRC=0b01111111;
	DDRD=0b00010110;
	PORTD|=(1<<PORTD3)|(1<<PORTD0);
	PORTB|=0b01100011;
	
	//--------Настройка таймера 2-------------------------------------
	TCCR2A = 0b00000000;        // нормальный режим
	TCCR2B = 0b00000100;        // предделитель CLK/64;
	TIMSK2=OFF;		// прерывание по переполнению
	
	TCCR1A = 0b00000000;        // нормальный режим
	TCCR1B = 0b00000011;        // предделитель CLK/64;
	TCCR1C = 0b00000000;		//
	TIMSK1=OFF;		// прерывание по переполнению

	
	//--------Настройка таймера 0-------------------------------------
	TCCR0A = 0b00000000;        // нормальный режим
	TCCR0B = 0b00000100;        // предделитель CLK/64;
	TIMSK0 = ON;		// прерывание по переполнению	

	_delay_ms(500);
	
	PU_mode=(PIND>>PIND3)&1;
	
	if(!PU_mode){
		admin_password[0]=read_eeprom(0x50);
		admin_password[1]=read_eeprom(0x51);
		admin_password[2]=read_eeprom(0x52);
		admin_password[3]=read_eeprom(0x53);

		user_password[0]=read_eeprom(0x60);
		user_password[1]=read_eeprom(0x61);
		user_password[2]=read_eeprom(0x62);
		user_password[3]=read_eeprom(0x63);
	
		for(uint8_t i=0;i<64;i++){
			EMZ.status[i]=read_eeprom(i);
		}
	}
	
	sei();
	
	lcd_LED(ON);
	lcd_init();
	SetLOGO();
	
	update=1;
	
	
	USART_init();
	
	transmit=1;
	
    while (1) 
    {
		//если режим ДПНК
		if(PU_mode){
			if(((package[73]<<4)|package[74])==STOP_BYTE){
				CRC_IN=0x07;
				for(uint8_t i=0;i<71;i++){
					CRC_IN^=package[i+1];
				}
				if(CRC_IN==package[72]){
					if((package[1]-0x30)*10+(package[2]-0x30)==32){
						
						for(uint8_t i=0;i<64;i++){
							EMZ.status[i]=package[i+3];
						}
						DPNK_BLOCK=package[67];
						if(DPNK_BLOCK==BLOCK){
							open_EMZ=BLOCK;
						}
						user_password[0]=package[68];
						user_password[1]=package[69];
						user_password[2]=package[70];
						user_password[3]=package[71];
	
						package[1]=0;
						package[2]=0;
						package[73]=0;
						package[74]=0;
						timer_DPNK=5;			//время ожидания ответа
						TIMSK1 = ON;
						UART_byte=0;
						transmit=1;
					}
				}
			}
			if(!timer_DPNK){
				if(page!=5){
					page=5;
					update=1;
				}
			}
			else{
				if(page==5){
					page=0;
					update=1;
				}
			}
			if(transmit){
				switch(UART_byte){
					case 0:
						RS485_ON;
						UCSR0B &= ~(1<<RXCIE0);
						UART_byte++;
						CRC=7;
						break;
					case 1:
						if(UCSR0A & (1<<UDRE0)){		//если передатчик UART0 свободен
							UDR0=START_BYTE;			//отправка СТАРТ-БАЙТА
							UART_byte++;				//инкрементирование индекса пакетов
						}
						break;
					case 2:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=0x33;
							UDR0=0x33;
							UART_byte++;
						}
						break;
					case 3:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=0x32;
							UDR0=0x32;
							UART_byte++;
						}
						break;
					case 4:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(sistem_mode+0x30);
							UDR0=sistem_mode+0x30;
							UART_byte++;
						}
						break;
					case 5:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=open_EMZ/10+0x30;
							UDR0=open_EMZ/10+0x30;
							UART_byte++;
						}
						break;
					case 6:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=open_EMZ%10+0x30;
							UDR0=open_EMZ%10+0x30;
							UART_byte++;
						}
						break;
					case 7:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=CRC;
							UART_byte++;
						}
						break;
					case 8:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0D;
							UART_byte++;
						}
						break;
					case 9:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0A;
							UART_byte++;
						}
						break;
					case 10:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x00;
							UART_byte++;
						}
						break;
					case 11:
						if(UCSR0A & (1<<UDRE0)){
							RS485_OFF;
							UCSR0B |= (1<<RXCIE0);
							transmit=0;
						}
						break;
					default:;
				}
			}
		}
//------------------------------------------------------------------------------------------------------------------
//если режим ДК
		else{
			if(((package[6]<<4)|package[7])==STOP_BYTE){
				CRC_IN=0x07;
				for(uint8_t i=1;i<5;i++){
					CRC_IN^=package[i];
				}
				lcd_set_ch(package[1],0,0);
				lcd_set_ch(package[2],0,1);
				lcd_set_ch(package[3],0,2);
				lcd_set_ch(package[4],0,3);
				lcd_set_ch(package[5]+0x30,0,4);
				lcd_set_ch(package[6]+0x30,0,5);
				lcd_set_ch(package[7]+0x30,0,6);
				lcd_set_ch(CRC+0x30,0,7);
				if(CRC_IN==package[5]){
					if(((package[1]-0x30)*10)+(package[2]-0x30)==UART_adress){
						EMZ.status[UART_adress*2]=package[3];
						if(package[3]==OPEN){
							DPNK_BLOCK=BLOCK;
							EMZ.out[UART_adress*2]=BLOCK;
						}
						EMZ.status[UART_adress*2+1]=package[4];
						if(package[4]==OPEN){
							DPNK_BLOCK=BLOCK;
							EMZ.out[UART_adress*2+1]=BLOCK;
						}
						EMZ.error[UART_adress]=0;
						package[1]=0;
						package[2]=0;
						package[6]=0;
						package[7]=0;
						TIMSK2=OFF;
						UART_byte=0;
						transmit=0;
					}
				}
			}
//если ответ от ПУ ДПНК
			else if(((package[7]<<4)|package[8])==STOP_BYTE){
				CRC_IN=7;
				for(uint8_t i=0;i<6;i++){
					CRC_IN^=package[i+1];
				}
				if(CRC_IN==package[6]){
					if(((package[1]-30)*10)+(package[2]-30)==0x32){
						sistem_mode=package[3]-0x30;
						if(package[4]!=OPEN){
							EMZ.out[(package[4]-0x30)*10+package[5]-0x30]=OPEN;
							DPNK_BLOCK=BLOCK;
						}
						if(page==5){
							page=0;
							update=1;
						}
						package[1]=0;
						package[2]=0;
						package[7]=0;
						package[8]=0;
						TIMSK2=OFF;
						UART_byte=0;
						transmit=0;
					}
				}
			}
			if(transmit){
				switch(UART_byte){
					case 0:						//включение передатчика
						
						UCSR0B&=~(1<<RXCIE0);
						package[1]=0;
						package[2]=0;
						package[6]=0;
						package[7]=0;
						package[8]=0;
						package[73]=0;
						package[74]=0;
						UART_byte=100;
						CRC=7;
						break;
					case 100:						//посылка СТАРТ-байта
						if(UCSR0A & (1<<UDRE0)){		//если передатчик UART0 свободен
							UDR0=0;			//отправка СТАРТ-БАЙТА
							UART_byte++;				//инкрементирование индекса пакетов
						}
						break;
					case 101:						//посылка СТАРТ-байта
						if(UCSR0A & (1<<UDRE0)){		//если передатчик UART0 свободен
							UDR0=0;			//отправка СТАРТ-БАЙТА
							UART_byte++;				//инкрементирование индекса пакетов
						}
						break;
					case 102:						//посылка СТАРТ-байта
						if(UCSR0A & (1<<UDRE0)){		//если передатчик UART0 свободен
							UDR0=0;			//отправка СТАРТ-БАЙТА
							UART_byte=1;				//инкрементирование индекса пакетов
						}
						break;
					case 1:						//посылка СТАРТ-байта
						if(UCSR0A & (1<<UDRE0)){		//если передатчик UART0 свободен
							RS485_ON;
							UDR0=START_BYTE;			//отправка СТАРТ-БАЙТА
							UART_byte++;				//инкрементирование индекса пакетов
						}
						break;
					case 2:						//посылка старшего разряда адреса
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(UART_adress/10+0x30);
							UDR0=(UART_adress/10+0x30);
							UART_byte++;
						}
						break;
					case 3:						//посылка младшего разряда адреса
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(UART_adress%10+0x30);
							UDR0=(UART_adress%10+0x30);
							if(UART_adress==32)UART_byte=6;
							//else if(UART_adress==80)UART_byte++;
							else UART_byte++;
						}
						break;
					case 4:						//посылка команды на замок 1
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(EMZ.out[UART_adress*2]);
							UDR0=EMZ.out[UART_adress*2];
							UART_byte++;
						}
						break;
					case 5:						//посылка команды на замок 2
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(EMZ.out[UART_adress*2+1]);
							UDR0=EMZ.out[UART_adress*2+1];
							UART_byte=77;
						}
						break;
					case 6:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[0];
							UDR0=EMZ.status[0];
							UART_byte++;
						}
						break;
					case 7:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[1];
							UDR0=EMZ.status[1];
							UART_byte++;
						}
						break;
					case 8:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[2];
							UDR0=EMZ.status[2];
							UART_byte++;
						}
						break;
					case 9:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[3];
							UDR0=EMZ.status[3];
							UART_byte++;
						}
						break;
					case 10:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[4];
							UDR0=EMZ.status[4];
							UART_byte++;
						}
						break;
					case 11:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[5];
							UDR0=EMZ.status[5];
							UART_byte++;
						}
						break;
					case 12:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[6];
							UDR0=EMZ.status[6];
							UART_byte++;
						}
						break;
					case 13:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[7];
							UDR0=EMZ.status[7];
							UART_byte++;
						}
						break;
					case 14:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[8];
							UDR0=EMZ.status[8];
							UART_byte++;
						}
						break;
					case 15:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[9];
							UDR0=EMZ.status[9];
							UART_byte++;
						}
						break;
					case 16:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[10];
							UDR0=EMZ.status[10];
							UART_byte++;
						}
						break;
					case 17:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[11];
							UDR0=EMZ.status[11];
							UART_byte++;
						}
						break;
					case 18:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[12];
							UDR0=EMZ.status[12];
							UART_byte++;
						}
						break;
					case 19:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[13];
							UDR0=EMZ.status[13];
							UART_byte++;
						}
						break;
					case 20:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[14];
							UDR0=EMZ.status[14];
							UART_byte++;
						}
						break;
					case 21:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[15];
							UDR0=EMZ.status[15];
							UART_byte++;
						}
						break;
					case 22:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[16];
							UDR0=EMZ.status[16];
							UART_byte++;
						}
						break;
					case 23:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[17];
							UDR0=EMZ.status[17];
							UART_byte++;
						}
						break;
					case 24:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[18];
							UDR0=EMZ.status[18];
							UART_byte++;
						}
						break;
					case 25:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[19];
							UDR0=EMZ.status[19];
							UART_byte++;
						}
						break;
					case 26:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[20];
							UDR0=EMZ.status[20];
							UART_byte++;
						}
						break;
					case 27:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[21];
							UDR0=EMZ.status[21];
							UART_byte++;
						}
						break;
					case 28:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[22];
							UDR0=EMZ.status[22];
							UART_byte++;
						}
						break;
					case 29:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[23];
							UDR0=EMZ.status[23];
							UART_byte++;
						}
						break;
					case 30:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[24];
							UDR0=EMZ.status[24];
							UART_byte++;
						}
						break;
					case 31:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[25];
							UDR0=EMZ.status[25];
							UART_byte++;
						}
						break;
					case 32:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[26];
							UDR0=EMZ.status[26];
							UART_byte++;
						}
						break;
					case 33:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[27];
							UDR0=EMZ.status[27];
							UART_byte++;
						}
						break;
					case 34:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[28];
							UDR0=EMZ.status[28];
							UART_byte++;
						}
						break;
					case 35:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[29];
							UDR0=EMZ.status[29];
							UART_byte++;
						}
						break;
					case 36:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[30];
							UDR0=EMZ.status[30];
							UART_byte++;
						}
						break;
					case 37:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[31];
							UDR0=EMZ.status[31];
							UART_byte++;
						}
						break;
					case 38:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[32];
							UDR0=EMZ.status[32];
							UART_byte++;
						}
						break;
					case 39:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[33];
							UDR0=EMZ.status[33];
							UART_byte++;
						}
						break;
					case 40:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[34];
							UDR0=EMZ.status[34];
							UART_byte++;
						}
						break;
					case 41:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[35];
							UDR0=EMZ.status[35];
							UART_byte++;
						}
						break;
					case 42:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[36];
							UDR0=EMZ.status[36];
							UART_byte++;
						}
						break;
					case 43:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[37];
							UDR0=EMZ.status[37];
							UART_byte++;
						}
						break;
					case 44:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[38];
							UDR0=EMZ.status[38];
							UART_byte++;
						}
						break;
					case 45:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[39];
							UDR0=EMZ.status[39];
							UART_byte++;
						}
						break;
					case 46:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[40];
							UDR0=EMZ.status[40];
							UART_byte++;
						}
						break;
					case 47:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[41];
							UDR0=EMZ.status[41];
							UART_byte++;
						}
						break;
					case 48:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[42];
							UDR0=EMZ.status[42];
							UART_byte++;
						}
						break;
					case 49:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[43];
							UDR0=EMZ.status[43];
							UART_byte++;
						}
						break;
					case 50:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[44];
							UDR0=EMZ.status[44];
							UART_byte++;
						}
						break;
					case 51:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[45];
							UDR0=EMZ.status[45];
							UART_byte++;
						}
						break;
					case 52:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[46];
							UDR0=EMZ.status[46];
							UART_byte++;
						}
						break;
					case 53:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[47];
							UDR0=EMZ.status[47];
							UART_byte++;
						}
						break;
					case 54:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[48];
							UDR0=EMZ.status[48];
							UART_byte++;
						}
						break;
					case 55:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[49];
							UDR0=EMZ.status[49];
							UART_byte++;
						}
						break;
					case 56:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[50];
							UDR0=EMZ.status[50];
							UART_byte++;
						}
						break;
					case 57:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[51];
							UDR0=EMZ.status[51];
							UART_byte++;
						}
						break;
					case 58:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[52];
							UDR0=EMZ.status[52];
							UART_byte++;
						}
						break;
					case 59:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[53];
							UDR0=EMZ.status[53];
							UART_byte++;
						}
						break;
					case 60:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[54];
							UDR0=EMZ.status[54];
							UART_byte++;
						}
						break;
					case 61:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[55];
							UDR0=EMZ.status[55];
							UART_byte++;
						}
						break;
					case 62:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[56];
							UDR0=EMZ.status[56];
							UART_byte++;
						}
						break;
					case 63:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[57];
							UDR0=EMZ.status[57];
							UART_byte++;
						}
						break;
					case 64:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[58];
							UDR0=EMZ.status[58];
							UART_byte++;
						}
						break;
					case 65:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[59];
							UDR0=EMZ.status[59];
							UART_byte++;
						}
						break;
					case 66:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[60];
							UDR0=EMZ.status[60];
							UART_byte++;
						}
						break;
					case 67:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[61];
							UDR0=EMZ.status[61];
							UART_byte++;
						}
						break;
					case 68:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[62];
							UDR0=EMZ.status[62];
							UART_byte++;
						}
						break;
					case 69:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[63];
							UDR0=EMZ.status[63];
							UART_byte++;
						}
						break;
					case 70:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=DPNK_BLOCK;
							UDR0=DPNK_BLOCK;
							UART_byte++;
						}
						break;
					case 71:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=user_password[0+0x30];
							UDR0=user_password[0+0x30];
							UART_byte++;
						}
						break;
					case 72:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=user_password[1+0x30];
							UDR0=user_password[1+0x30];
							UART_byte++;
						}
						break;
					case 73:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=user_password[2+0x30];
							UDR0=user_password[2+0x30];
							UART_byte++;
						}
						break;
					case 74:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=user_password[3+0x30];
							UDR0=user_password[3+0x30];
							UART_byte=77;
						}
						break;

					case 77:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=CRC;
							UART_byte++;
						}
						break;
					case 78:						//посылка СТОП 0x0D
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0D;
							UART_byte++;
						}
						break;
					case 79:						//посылка СТОП 0x0D
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0A;
							UART_byte++;
						}
						break;
					case 80:
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x00;
							UART_byte++;
						}
						break;
					case 81:
						if(UCSR0A & (1<<UDRE0)){
							RS485_OFF;
							UCSR0B |= (1<<RXCIE0);
							timer=6;			//время ожидания ответа
							TIMSK2 = ON;
							UART_byte++;
						}
						break;
					case 82:
						if(!timer)UART_byte++;
						break;
					case 83:
						if(UART_adress<32){
							if(EMZ.error[UART_adress/2]==5){
								if(EMZ.status[UART_adress*2]!=DISCONNECT){
									EMZ.status[UART_adress*2]=ERROR;
									EMZ.out[UART_adress*2]=BLOCK;
								}
								if(EMZ.status[UART_adress*2+1]!=DISCONNECT){
									EMZ.status[UART_adress*2+1]=ERROR;
									EMZ.out[UART_adress*2+1]=BLOCK;
								}
							}
							else EMZ.error[UART_adress/2]++;
						}
						if(UART_adress==32){
							if(sistem_mode){
								if(DPNK_ERROR<5)DPNK_ERROR++;
								else {
									if(page!=5&&page!=2&&page!=22&&page!=10&&page!=21&&page!=4&&page!=41&&page!=42&&page!=43&&page!=44){
										page=5;
										update=1;
									}
								}
							}
						}
						UART_byte=0;
						transmit=0;
						break;
					default:;
				}
			}
			else{
				if(UART_adress==32)UART_adress=0;
				else UART_adress++;
				if(UART_adress<32){
					if(EMZ.status[UART_adress*2]!=DISCONNECT)transmit=1;
					else if(EMZ.status[UART_adress*2+1]!=DISCONNECT)transmit=1;
				}
				else {
					DPNK_BLOCK=OPEN;
					for(uint8_t i=0;i<64;i++){
						if(EMZ.status[i]==OPEN)DPNK_BLOCK=BLOCK;
					}
					transmit=1;
				}
			}
		}
		//чтение кнопок
		if(!BUT_FIX){
			BUTTON=scan_buttons();
			PORTB|=0b00011100;
			if(BUTTON<15){
				PORTC|=(1<<PORTC0);
				BUT_FIX=1;
				beep_time=20;
				//timer=2;
				TIMSK0=ON;
			}
		}
		else if(BUT_FIX>1){
			if(!beep_time){
				if(scan_buttons()==15)BUT_FIX=(BUT_FIX+1)%20;
				else BUT_FIX=2;
				PORTB|=0b00011100;
			}
		}
		update=0;
		//переключение страниц
		switch(page){
			case 0:Set_MAIN();break;
			case 1:Set_EMZ();break;
			case 2:Set_password();break;
			case 22:Error_password();break;
			case 21:Reset_pass();break;
			case 3:EMZ_info();break;
			case 4:Seting_menu();break;
			case 41:Set_password_admin();break;
			case 42:Set_password_user();break;
			case 43:Activate_EMZ();break;
			case 44:Scan_EMZ();break;
			case 5:Error_wireles();break;
			case 10:SetROJER();break;
			default:;
		}
    }
}

