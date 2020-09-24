/*
 * Functions.h
 *
 * Created: 17.02.2020 23:27:41
 *  Author: bayar
 */ 


#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_





#endif /* FUNCTIONS_H_ */

unsigned char scan_buttons(void){
	uint8_t but=0;
	PORTB&=~(1<<PORTB2);
	_delay_loop_1(2);
	but=PINB&0b01100011;
	if(but!=0b01100011){
		switch(but){
			case 0b01100001:return 1;break;
			case 0b01100010:return 4;break;
			case 0b00100011:return 7;break;
			case 0b01000011:return 10;break;
			default:;
		}
	}
	PORTB|=(1<<PORTB2);
	PORTB&=~(1<<PORTB3);
	_delay_loop_1(2);
	but=PINB&0b01100011;
	if(but!=0b01100011){
		switch(but){
			case 0b01100001:return 2;break;
			case 0b01100010:return 5;break;
			case 0b00100011:return 8;break;
			case 0b01000011:return 0;break;
			default:;
		}
	}
	PORTB|=(1<<PORTB3);
	PORTB&=~(1<<PORTB4);
	_delay_loop_1(2);
	but=PINB&0b01100011;
	if(but!=0b01100011){
		switch(but){
			case 0b01100001:return 3;break;
			case 0b01100010:return 6;break;
			case 0b00100011:return 9;break;
			case 0b01000011:return 11;break;
			default:;
		}
	}
	return 15;
}

void beep(uint8_t onoff){
	if(onoff)PORTC|=(1<<PORTC0);
	else PORTC&=~(1<<PORTC0);
}
void write_eeprom(uint8_t adr,uint8_t value){
	cli();
	while (EECR & (1<<EEPE));
	EEAR = adr;
	EEDR = value;
	EECR |= (1<<EEMPE);
	EECR |= (1<<EEPE);
	sei();
}

uint8_t read_eeprom(uint8_t adr){
	cli();
	while (EECR & (1<<EEPE));
	EEAR = adr;
	EECR |= (1<<EERE);
	sei();
	return EEDR;
}


/*
uint16_t return_CRC(uint16_t crc,uint8_t num){
	uint8_t crc_l=0;
	crc^=num;
	if(num&1){
		crc_l=crc>>8;
		crc=(crc<<8)|crc_l;
	}
	return(crc);
}*/

