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
#define FULL 0x46
#define LOW 0x4C

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

volatile uint8_t timer=0;					
volatile uint8_t sistem_mode=1;				//режим работы системы 0-ДПНК откл.; 1-режим 1; 2-режим 2
volatile uint8_t PU_mode=0;					//режим работы пульта 1-ДПНК;0-ДК
volatile uint8_t BUT_FIX=0;					//аиксация нажатия кнопки 0-кнопка не нажата, 1-кнопка нажата но не обработана, 2 кнопка обработана
volatile uint8_t BUTTON=15;					//номер нажатой кнопки 0-1, 11-*,12-#
volatile uint8_t page=0;					//номер страницы меню
volatile uint8_t update=0;					//обновление экрана 0-обновление запрещено, 1-обновление разрешено
volatile uint8_t big_digit_1=10;			//большая цифра 1
volatile uint8_t big_digit_2=10;			//большая цифра 2
volatile uint8_t DPNK_BLOCK=BLOCK;			//блокировка открывания замка пультом DPNK OPEN-разрешено, BLOCK-запрещено
volatile uint8_t beep_time=0;				//время бипера
volatile uint8_t transmit=0;				//разрешение пердачи по RS485 0-передача запрещена, 1-передача разрешена
volatile uint8_t UART_byte=0;				//счетчик отправки байтов по RS485
volatile uint8_t UART_adress=0;				//адрес устройства обмена данными 32-первая половина пакета ПУ ДПНК 64-вторая половина пакета ПУ ДПНК
volatile uint8_t bufer=0;					//буфер приема данных
volatile uint8_t package[80];				//массив принятых данных
volatile uint8_t RX_index=0;				//счетчик принятых байтов
volatile uint8_t CRC=0;						//CRC
volatile uint8_t open_EMZ=32;				//номер замка который необходимо открыть 
volatile uint8_t DPNK_ERROR=0;				//связь с DPNK 0-связь есть, 1 связи нет
volatile uint8_t user_password[4]={0,0,0,0};//пароль пользователя
volatile uint8_t admin_password[4]={0,0,0,0};//пароль админа
volatile uint8_t bufer_password[4]={10,10,10,10};//буфер ввода пароля
volatile uint8_t reset_password=0;			//счетчик нажатий для сброса паролей
volatile uint8_t timer_DPNK=0;				//
volatile uint8_t DPNK_wireles=0;			//
volatile uint8_t scan_on=0;		
volatile uint8_t scan_index=0;
volatile uint8_t password_error=0;			
volatile uint8_t CRC_IN=0;
volatile uint8_t lcd_bufer[5];				//буфер хранения временных данных для страниц интерфейса
volatile uint8_t duble_open=0;			
volatile uint8_t alarm_time=0;				//время срабатывания СЗУ
volatile uint8_t alarm=OFF;					
volatile uint8_t alarm_timer_off=0;			
volatile uint8_t send_EMZ=0;				
volatile uint8_t EMZ_map=0;
volatile uint8_t LCD_led_timer=0;
volatile uint8_t led_use=0;



//структура хранения данных о состоянии замков, конденсаторов, адресов
struct EMZ
{
	uint8_t status[64];
	uint8_t out[64];
	uint8_t error[32];
	uint8_t cap[64];
	uint8_t adress[64];
	uint8_t position[64];
	}EMZ={
	{BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK},
	{BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK,BLOCK},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW},
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64},
	{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64}
};

#include "USART.h"
#include "lcd128x64.h"
#include "Functions.h"
#include "LCDmenu.h"


//прерывание по приему UART
ISR(USART0_RX_vect){
	bufer=UDR0;
	if(bufer==START_BYTE){
		package[1]=0;
		package[2]=0;
		package[7]=0;
		package[8]=0;
		package[9]=0;
		package[74]=0;
		package[75]=0;
		RX_index=0;
	}
	if(RX_index<76){
		package[RX_index]=bufer;
		RX_index++;
	}
}

//прерывание по таймеру 0
ISR(TIMER0_OVF_vect){
	if(beep_time){
		beep_time--;
		if(beep_time==10)PORTC&=~(1<<PORTC0);
	}
	else{
		if(alarm==ON){
			PORTC|=(1<<PORTC0);
			beep_time=20;
		}
	}
}


//прерывание по таймеру 1
ISR (TIMER1_OVF_vect){
	if(LCD_led_timer){
		LCD_led_timer--;
	}
	else if(!led_use)led_use=1;
	if(PU_mode==DK){
		if(alarm_timer_off)alarm_timer_off--;
		else{
			alarm=OFF;
			PORTD&=~(1<<PORTD4);
		}
	}
	if(timer_DPNK)timer_DPNK--;
	TCNT1=3035;
}

//прерывание по таймеру 2
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
	PORTD=0b00001001;
	PORTB|=0b01100011;
	
	//--------Настройка таймера 2-------------------------------------
	TCCR2A = 0b00000000;        // нормальный режим
	TCCR2B = 0b00000100;        // предделитель
	TIMSK2=OFF;					
	
	TCCR1A = 0b00000000;        // нормальный режим
	TCCR1B = 0b00000011;        // предделитель
	TCCR1C = 0b00000000;		//
	TIMSK1=ON;					

	
	//--------Настройка таймера 0-------------------------------------
	TCCR0A = 0b00000000;        // нормальный режим
	TCCR0B = 0b00000100;        // предделитель
	TIMSK0 = ON;		

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
		
		alarm_time=read_eeprom(0x70);
		sistem_mode=read_eeprom(0x71);
		
		for(uint8_t i=0;i<64;i++){
			EMZ.status[i]=read_eeprom(i);
			EMZ.adress[i]=read_eeprom(i+0x80);
			EMZ.position[i]=read_eeprom(i+0xC0);
		}
	}
	
	sei();						//запрет прерываний
	PORTC|=(1<<PORTC0);			
	beep_time=20;				//звуковой сигнал
	lcd_LED(ON);				//включение подсветки дисплея
	lcd_init();					//инициализация дисплея
	SetLOGO();					//вывод логотипа
	
	update=1;
	
	
	USART_init();				//инициализация UART
	UART_byte=0;
	if(PU_mode)	transmit=0;
	else transmit=1;
	RS485_OFF;
	
	while (1)
	{
		if(PU_mode==DPNK){
			if(!transmit){
				if(!timer_DPNK){
					if(page!=5)update=1;
					page=5;
				}
				else if(page==5){
					page=0;
					update=1;
				}
				if(((package[74]<<4)|package[75])==STOP_BYTE){
					CRC_IN=0x07;
					for(uint8_t i=1;i<73;i++){
						CRC_IN^=package[i];
					}
					if(CRC_IN==package[73]){
						if(((package[1]-0x30)*10)+(package[2]-0x30)==32){
							if(package[67]==DISCONNECT){
								if(page!=7)update=1;
								page=7;
								if(sistem_mode){
									update=1;
									sistem_mode=0;
								}
								timer_DPNK=5;
							}
							else{
								if(!sistem_mode)sistem_mode=1;
								for(uint8_t i=0;i<32;i++){
									EMZ.status[i]=package[i*2+3];
									EMZ.cap[i]=package[i*2+4];
								}
								DPNK_BLOCK=package[67];
								if(DPNK_BLOCK==BLOCK)open_EMZ=32;
								user_password[0]=package[69]-0x30;
								user_password[1]=package[70]-0x30;
								user_password[2]=package[71]-0x30;
								user_password[3]=package[72]-0x30;
								alarm=package[68]-0x30;
								if(alarm){
									PORTD|=(1<<PORTD4);
								}
								else{
									PORTD&=~(1<<PORTD4);
								}
								timer_DPNK=5;
								transmit=1;
								UART_byte=0;
							}
						}
						else if(((package[1]-0x30)*10)+(package[2]-0x30)==64){
							if(sistem_mode){
								if(open_EMZ<32){
									if(package[67]==open_EMZ)open_EMZ=32;
								}
								for(uint8_t i=0;i<32;i++){
									EMZ.status[i+32]=package[i*2+3];
									EMZ.cap[i+32]=package[i*2+4];
									transmit=1;
									UART_byte=0;
									timer_DPNK=5;
								}
							}
						}
					}
					package[1]=0;
					package[2]=0;
					package[74]=0;
					package[75]=0;
				}	
			}
			if(transmit){
				switch (UART_byte){
					case 0:								//переключение в состояние передачи сигнала
						UCSR0B&=~(1<<RXCIE0);
						CRC=7;
						UART_byte++;
						break;
					case 1:								// продувка линии
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=0;						
							UART_byte++;				
						}
						break;
					case 2:								// продувка линии
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=0;						
							UART_byte++;				
						}
						break;
					case 3:								// продувка линии
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=0;						
							RS485_ON;
							UART_byte++;				
						}
						break;
					case 4:								// отправка СТАРТ-БАЙТА
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=START_BYTE;			
							UART_byte++;				
						}
						break;
					case 5:								//отправка старшего разряда адреса
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(0x33);
							UDR0=(0x33);
							UART_byte++;
						}
						break;
					case 6:								//отправка младшего разряда адреса
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(0x32);
							UDR0=(0x32);
							UART_byte++;
						}
						break;
					case 7:								//отправка режима работы
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(sistem_mode+0x30);
							UDR0=(sistem_mode+0x30);
							UART_byte++;
						}
						break;
					case 8:								//отправка старшего разряда адреса для открывания замка
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(open_EMZ/10+0x30);
							UDR0=(open_EMZ/10+0x30);
							UART_byte++;
						}
						break;
					case 9:								//отправка младшего разряда адреса для открывания замка
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(open_EMZ%10+0x30);
							UDR0=(open_EMZ%10+0x30);
							UART_byte++;
						}
						break;
					case 10:							//отправка CRC
						if(UCSR0A & (1<<UDRE0)){
							UDR0=CRC;
							UART_byte++;
						}
						break;
					case 11:						//посылка СТОП-БАЙТА 0x0D
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0D;
							UART_byte++;
						}
						break;
					case 12:						//посылка СТОП-БАЙТА 0x0A
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x0A;
							UART_byte++;
						}
						break;
					case 13:						// продувка линии
						if(UCSR0A & (1<<UDRE0)){
							UDR0=0x00;
							UART_byte++;
						}
						break;
					case 14:						// выключение передатчика
						if(UCSR0A & (1<<UDRE0)){
							RS485_OFF;
							UCSR0B |= (1<<RXCIE0);
							transmit=0;
							UART_byte=0;
						}
						break;
						default:transmit=0;
				}
			}
		}
		else{
			if(transmit){
				switch (UART_byte){
					case 0:
						UCSR0B&=~(1<<RXCIE0);
						CRC=7;
						UART_byte=100;
						break;
					case 100:
						if(UCSR0A & (1<<UDRE0)){	
							UDR0=0;						
							UART_byte++;				
						}
						break;
					case 101:
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=0;						
							UART_byte++;				
						}
						break;	
					case 102:
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=0;						
							RS485_ON;
							UART_byte=1;				
						}
						break;	
							
					case 1:
						if(UCSR0A & (1<<UDRE0)){		
							UDR0=START_BYTE;			
							UART_byte++;				
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
							if(UART_adress>31)UART_byte=6;
							//else if(UART_adress==80)UART_byte++;
							else UART_byte++;
						}
						break;
					case 4:						//посылка команды на замок 1
						if(UCSR0A & (1<<UDRE0)){
							EMZ_map=EMZ.adress[UART_adress*2];
							CRC^=(EMZ.out[EMZ_map]);
							UDR0=EMZ.out[EMZ_map];
							UART_byte++;
						}
						break;
					case 5:						//посылка команды на замок 2
						if(UCSR0A & (1<<UDRE0)){
							EMZ_map=EMZ.adress[UART_adress*2+1];
							CRC^=(EMZ.out[EMZ_map]);
							UDR0=EMZ.out[EMZ_map];
							UART_byte=77;
						}
						break;
					case 6:									//ЭМЗ-1/33
						if(UCSR0A & (1<<UDRE0)){
							
							CRC^=EMZ.status[UART_adress-32];
							UDR0=EMZ.status[UART_adress-32];
							UART_byte++;
						}
						break;
					case 7:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-32];
							UDR0=EMZ.cap[UART_adress-32];
							UART_byte++;
						}
						break;
					case 8:									//ЭМЗ-2/34
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-31];
							UDR0=EMZ.status[UART_adress-31];
							UART_byte++;
						}
						break;
					case 9:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-31];
							UDR0=EMZ.cap[UART_adress-31];
							UART_byte++;
						}
						break;
					case 10:								//ЭМЗ-3/35
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-30];
							UDR0=EMZ.status[UART_adress-30];
							UART_byte++;
						}
						break;
					case 11:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-30];
							UDR0=EMZ.cap[UART_adress-30];
							UART_byte++;
						}
						break;
					case 12:							//ЭМЗ-4/36
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-29];
							UDR0=EMZ.status[UART_adress-29];
							UART_byte++;
						}
						break;
					case 13:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-29];
							UDR0=EMZ.cap[UART_adress-29];
							UART_byte++;
						}
						break;
					case 14:							//ЭМЗ-5/37
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-28];
							UDR0=EMZ.status[UART_adress-28];
							UART_byte++;
						}
						break;
					case 15:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-28];
							UDR0=EMZ.cap[UART_adress-28];
							UART_byte++;
						}
						break;
					case 16:							//ЭМЗ-6/38
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-27];
							UDR0=EMZ.status[UART_adress-27];
							UART_byte++;
						}
						break;
					case 17:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-27];
							UDR0=EMZ.cap[UART_adress-27];
							UART_byte++;
						}
						break;
					case 18:							//ЭМЗ-7/39
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-26];
							UDR0=EMZ.status[UART_adress-26];
							UART_byte++;
						}
						break;
					case 19:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-26];
							UDR0=EMZ.cap[UART_adress-26];
							UART_byte++;
						}
						break;
					case 20:							//ЭМЗ-8/40
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-25];
							UDR0=EMZ.status[UART_adress-25];
							UART_byte++;
						}
						break;
					case 21:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-25];
							UDR0=EMZ.cap[UART_adress-25];
							UART_byte++;
						}
						break;
					case 22:							//ЭМЗ-9/41
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-24];
							UDR0=EMZ.status[UART_adress-24];
							UART_byte++;
						}
						break;
					case 23:	
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-24];
							UDR0=EMZ.cap[UART_adress-24];
							UART_byte++;
						}
						break;
					case 24:							//ЭМЗ-10/42
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-23];
							UDR0=EMZ.status[UART_adress-23];
							UART_byte++;
						}
						break;
					case 25:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-23];
							UDR0=EMZ.cap[UART_adress-23];
							UART_byte++;
						}
						break;
					case 26:							//ЭМЗ-11/43
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-22];
							UDR0=EMZ.status[UART_adress-22];
							UART_byte++;
						}
						break;
					case 27:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-22];
							UDR0=EMZ.cap[UART_adress-22];
							UART_byte++;
						}
						break;
					case 28:							//ЭМЗ-12/44
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-21];
							UDR0=EMZ.status[UART_adress-21];
							UART_byte++;
						}
						break;
					case 29:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-21];
							UDR0=EMZ.cap[UART_adress-21];
							UART_byte++;
						}
						break;
					case 30:							//ЭМЗ-13/45
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-20];
							UDR0=EMZ.status[UART_adress-20];
							UART_byte++;
						}
						break;
					case 31:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-20];
							UDR0=EMZ.cap[UART_adress-20];
							UART_byte++;
						}
						break;
					case 32:							//ЭМЗ-14/46
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-19];
							UDR0=EMZ.status[UART_adress-19];
							UART_byte++;
						}
						break;
					case 33:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-19];
							UDR0=EMZ.cap[UART_adress-19];
							UART_byte++;
						}
						break;
					case 34:								//ЭМЗ-15/47
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-18];
							UDR0=EMZ.status[UART_adress-18];
							UART_byte++;
						}
						break;
					case 35:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-18];
							UDR0=EMZ.cap[UART_adress-18];
							UART_byte++;
						}
						break;
					case 36:							//ЭМЗ-16/48
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-17];
							UDR0=EMZ.status[UART_adress-17];
							UART_byte++;
						}
						break;
					case 37:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-17];
							UDR0=EMZ.cap[UART_adress-17];
							UART_byte++;
						}
						break;
					case 38:						//ЭМЗ-17/49
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-16];
							UDR0=EMZ.status[UART_adress-16];
							UART_byte++;
						}
						break;
					case 39:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-16];
							UDR0=EMZ.cap[UART_adress-16];
							UART_byte++;
						}
						break;
					case 40:						//ЭМЗ-18/50
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-15];
							UDR0=EMZ.status[UART_adress-15];
							UART_byte++;
						}
						break;
					case 41:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-15];
							UDR0=EMZ.cap[UART_adress-15];
							UART_byte++;
						}
						break;
					case 42:							//ЭМЗ-19/51
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-14];
							UDR0=EMZ.status[UART_adress-14];
							UART_byte++;
						}
						break;
					case 43:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-14];
							UDR0=EMZ.cap[UART_adress-14];
							UART_byte++;
						}
						break;
					case 44:							//ЭМЗ-20/52
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-13];
							UDR0=EMZ.status[UART_adress-13];
							UART_byte++;
						}
						break;
					case 45:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-13];
							UDR0=EMZ.cap[UART_adress-13];
							UART_byte++;
						}
						break;
					case 46:							//ЭМЗ-21/53
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-12];
							UDR0=EMZ.status[UART_adress-12];
							UART_byte++;
						}
						break;
					case 47:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-12];
							UDR0=EMZ.cap[UART_adress-12];
							UART_byte++;
						}
						break;
					case 48:							//ЭМЗ-22/54
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-11];
							UDR0=EMZ.status[UART_adress-11];
							UART_byte++;
						}
						break;
					case 49:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-11];
							UDR0=EMZ.cap[UART_adress-11];
							UART_byte++;
						}
						break;
					case 50:							//ЭМЗ-23/55
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-10];
							UDR0=EMZ.status[UART_adress-10];
							UART_byte++;
						}
						break;
					case 51:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-10];
							UDR0=EMZ.cap[UART_adress-10];
							UART_byte++;
						}
						break;
					case 52:							//ЭМЗ-24/56
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-9];
							UDR0=EMZ.status[UART_adress-9];
							UART_byte++;
						}
						break;
					case 53:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-9];
							UDR0=EMZ.cap[UART_adress-9];
							UART_byte++;
						}
						break;
					case 54:							//ЭМЗ-25/57
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-8];
							UDR0=EMZ.status[UART_adress-8];
							UART_byte++;
						}
						break;
					case 55:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-8];
							UDR0=EMZ.cap[UART_adress-8];
							UART_byte++;
						}
						break;
					case 56:							//ЭМЗ-26/58
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-7];
							UDR0=EMZ.status[UART_adress-7];
							UART_byte++;
						}
						break;
					case 57:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-7];
							UDR0=EMZ.cap[UART_adress-7];
							UART_byte++;
						}
						break;
					case 58:							//ЭМЗ-27/59
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-6];
							UDR0=EMZ.status[UART_adress-6];
							UART_byte++;
						}
						break;
					case 59:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-6];
							UDR0=EMZ.cap[UART_adress-6];
							UART_byte++;
						}
						break;
					case 60:							//ЭМЗ-28/60
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-5];
							UDR0=EMZ.status[UART_adress-5];
							UART_byte++;
						}
						break;
					case 61:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-5];
							UDR0=EMZ.cap[UART_adress-5];
							UART_byte++;
						}
						break;
					case 62:							//ЭМЗ-29/61
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-4];
							UDR0=EMZ.status[UART_adress-4];
							UART_byte++;
						}
						break;
					case 63:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-4];
							UDR0=EMZ.cap[UART_adress-4];
							UART_byte++;
						}
						break;
					case 64:							//ЭМЗ-30/62
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-3];
							UDR0=EMZ.status[UART_adress-3];
							UART_byte++;
						}
						break;
					case 65:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-3];
							UDR0=EMZ.cap[UART_adress-3];
							UART_byte++;
						}
						break;
					case 66:							//ЭМЗ-31/63
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-2];
							UDR0=EMZ.status[UART_adress-2];
							UART_byte++;
						}
						break;
					case 67:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-2];
							UDR0=EMZ.cap[UART_adress-2];
							UART_byte++;
						}
						break;
					case 68:						//ЭМЗ-32/64
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.status[UART_adress-1];
							UDR0=EMZ.status[UART_adress-1];
							UART_byte++;
						}
						break;
					case 69:							
						if(UCSR0A & (1<<UDRE0)){
							CRC^=EMZ.cap[UART_adress-1];
							UDR0=EMZ.cap[UART_adress-1];
							UART_byte++;
						}
						break;
					case 70:						
						if(UCSR0A & (1<<UDRE0)){
							if(UART_adress==32){
								if(!sistem_mode){
									CRC^=DISCONNECT;
									UDR0=DISCONNECT;
								}
								else{
									CRC^=DPNK_BLOCK;
									UDR0=DPNK_BLOCK;
								}
							}
							else{
								CRC^=open_EMZ;
								UDR0=open_EMZ;
								open_EMZ=32;
							}
							UART_byte++;
						}
						break;
					case 71:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(alarm+0x30);
							UDR0=(alarm+0x30);
							UART_byte++;
						}
						break;
					case 72:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(user_password[0]+0x30);
							UDR0=(user_password[0]+0x30);
							UART_byte++;
						}
						break;
					case 73:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(user_password[1]+0x30);
							UDR0=(user_password[1]+0x30);
							UART_byte++;
						}
						break;
					case 74:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(user_password[2]+0x30);
							UDR0=(user_password[2]+0x30);
							UART_byte++;
						}
						break;
					case 75:
						if(UCSR0A & (1<<UDRE0)){
							CRC^=(user_password[3]+0x30);
							UDR0=(user_password[3]+0x30);
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
						if(timer){
			//ответ от ЭМЗ
							if(((package[8]<<4)|package[9])==STOP_BYTE){
								CRC_IN=0x07;
								for(uint8_t i=1;i<7;i++){
									CRC_IN^=package[i];
								}
								if(CRC_IN==package[7]){
									if(((package[1]-0x30)*10)+(package[2]-0x30)==UART_adress){
										
										EMZ_map=EMZ.adress[UART_adress*2];
										if(EMZ.status[EMZ_map]!=DISCONNECT){
											if(EMZ.status[EMZ_map]==BLOCK){
												if(EMZ.status[EMZ_map]!=package[3]){
													if(alarm_time){
														alarm=ON;
														alarm_timer_off=alarm_time;
														PORTD|=(1<<PORTD4);
													}
												}
											}
											EMZ.status[EMZ_map]=package[3];
											if(package[3]==OPEN)DPNK_BLOCK=BLOCK;
											EMZ.cap[EMZ_map]=package[5];
											EMZ.out[EMZ_map]=BLOCK;
										}
										
										EMZ_map=EMZ.adress[UART_adress*2+1];
										if(EMZ.status[EMZ_map]!=DISCONNECT){
											if(EMZ.status[EMZ_map]==BLOCK){
												if(EMZ.status[EMZ_map]!=package[4]){
													if(alarm_time){
														alarm=ON;
														alarm_timer_off=alarm_time;
														PORTD|=(1<<PORTD4);
													}
												}
											}
											EMZ.status[EMZ_map]=package[4];
											if(package[4]==OPEN)DPNK_BLOCK=BLOCK;
											EMZ.cap[EMZ_map]=package[6];
											EMZ.out[EMZ_map]=BLOCK;
										}
										
										EMZ.error[UART_adress]=0;
										package[1]=0;
										package[2]=0;
										package[8]=0;
										package[9]=0;
										TIMSK2=OFF;
										UART_byte=0;
										transmit=0;
									}
								}
							}
				//ответ от ДПНК
							if(((package[7]<<4)|package[8])==STOP_BYTE){
								CRC_IN=0x07;
								for(uint8_t i=1;i<6;i++){
									CRC_IN^=package[i];
								}
								if(CRC_IN==package[6]){
									if(((package[1]-0x30)*10)+(package[2]-0x30)==32){
										if(sistem_mode)sistem_mode=package[3]-0x30;
										open_EMZ=(package[4]-0x30)*10+(package[5]-0x30);
										if(open_EMZ<32){
											EMZ.out[open_EMZ]=OPEN;
											//open_EMZ=32;
										}
										if(page==5){
											page=0;
											update=1;
										}
										DPNK_ERROR=0;
										package[1]=0;
										package[2]=0;
										package[7]=0;
										package[8]=0;
										TIMSK2=OFF;
										UART_byte=0;
										transmit=0;
									}
									else{
										if(((package[1]-0x30)*10)+(package[2]-0x30)==64){
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
							}
						}
						else UART_byte++;
						break;
					case 83:
						if(UART_adress<32){
							if(EMZ.error[UART_adress]==5){
								EMZ_map=EMZ.adress[UART_adress*2];
								if(EMZ.status[EMZ_map]!=DISCONNECT){
									EMZ.status[EMZ_map]=ERROR;
									EMZ.out[EMZ_map]=BLOCK;
								}
								EMZ_map=EMZ.adress[UART_adress*2+1];
								if(EMZ.status[EMZ_map]!=DISCONNECT){
									EMZ.status[EMZ_map]=ERROR;
									EMZ.out[EMZ_map]=BLOCK;
								}
							}
							else EMZ.error[UART_adress]++;
						}
						else {
							if(sistem_mode){
								if(DPNK_ERROR<5)DPNK_ERROR++;
								else{
									if(page!=5&&page/10!=2&&page!=2&&page/10!=4&&page!=4&&page!=10){
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
				if(UART_adress==32)UART_adress=64;
				else if(UART_adress==64)UART_adress=0;
				else UART_adress++;
				if(UART_adress<32){
					EMZ_map=EMZ.adress[UART_adress*2];
					if(EMZ.status[EMZ_map]!=DISCONNECT)transmit=1;
					else {
						EMZ_map=EMZ.adress[UART_adress*2+1];
						if(EMZ.status[EMZ_map]!=DISCONNECT)transmit=1;
					}
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
				lcd_LED(ON);
				//timer=2;
				//TIMSK0=ON;
			}
		}
		else if(BUT_FIX>1){
			if(!beep_time){
				if(scan_buttons()==15)BUT_FIX=(BUT_FIX+1)%20;
				else BUT_FIX=2;
				PORTB|=0b00011100;
			}
		}
		//переключение страниц
		if(led_use==1){
			lcd_LED(OFF);
			page=0;
			update=1;
			led_use=2;
		}
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
			case 44:Adresing_EMZ();break;
			case 45:Adresing_EMZ_2();break;
			case 46:Reset_seting();break;
			case 5:Error_wireles();break;
			case 6:Set_sistem_mode();break;
			case 7:DPNK_DISCONNECT();break;
			case 10:SetROJER();break;
			default:;
		}
	}
}