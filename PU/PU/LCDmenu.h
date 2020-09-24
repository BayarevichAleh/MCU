/*
 * LCDmenu.h
 *
 * Created: 20.10.2019 9:33:49
 *  Author: bayar
 */ 


#ifndef LCDMENU_H_
	#define LCDMENU_H_
#endif /* LCDMENU_H_ */

#ifndef DPNK
	#define DPNK 0
#endif
#ifndef DK
	#define DK 1
#endif
#ifndef BLOCK
	#define BLOCK 0x42
#endif
#ifndef OPEN
	#define OPEN 0x4F
#endif
#ifndef DISCONNECT
	#define DISCONNECT 0x44
#endif
#ifndef ERROR
	#define ERROR 0x45
#endif
#ifndef FULL
	#define FULL 0x46
#endif
#ifndef LOW
	#define LOW 0x4C
#endif




#ifndef LCD128X64_H_
	#include "lcd128x64.h"
#endif



//Вывод логотипа
void SetLOGO(void){
	lcd_clean();
	for(unsigned char i=0;i<8;i++){
		WriteXY(32,i,1);
			WriteXY(0,i,2);
		for(unsigned char j=0;j<32;j++){
			WriteData(pgm_read_byte(&(LOGO[i][j])), 1);
			WriteData(pgm_read_byte(&(LOGO[i][j+32])), 2);
		}
	}
}

//page=10 "Вывод Роджера"
void SetROJER(void){
	if(update){
		lcd_clean();
		for(unsigned char i=0;i<8;i++){
			WriteXY(30,i,1);
			WriteXY(0,i,2);
			for(unsigned char j=0;j<34;j++){
				WriteData(pgm_read_byte(&(ROJER[i][j])), 1);
				WriteData(pgm_read_byte(&(ROJER[i][j+34])), 2);
			}
		}
		update=0;
	}
}

//page=0	"Основное меню"
void Set_MAIN(void){
	if(update){
		lcd_bufer[0]=sistem_mode;
		SetLOGO();
		if(PU_mode){
			lcd_set_string("ПУ",12,3);
			lcd_set_string("ДПНК",6,4);
		}
		else lcd_set_string("ПУ ДК",6,4);
		lcd_set_string("Режим",98,3);
		if(sistem_mode==1)lcd_set_ch(0x31,110,4);
		else lcd_set_ch(0x32,110,4);
		update=0;
	}
	if(lcd_bufer[0]!=sistem_mode)update=1;
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:page=2;break;
			case 11:break;
			default:page=1;big_digit_1=BUTTON;big_digit_2=10;break;
		}
		BUT_FIX=2;
		update=1;
	}
}

//page=1		меню ввода номера ЭМЗ
void Set_EMZ(){
	if(update){
		lcd_bufer[0]=sistem_mode;
		lcd_clean();
		if(sistem_mode==1)lcd_set_string("Режим 1",42,0);
		else lcd_set_string("Режим 2",42,0);
		if(big_digit_1<10)lcd_set_big_digit(big_digit_1,1);
		if(big_digit_2<10)lcd_set_big_digit(big_digit_2,2);
		lcd_set_string("*-Отмена",0,7);
		update=0;
	}
	if(lcd_bufer[0]!=sistem_mode)update=1;
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:page=0;break;
			case 11:break;
			default:
				big_digit_2=BUTTON;
				if(((big_digit_1*10+big_digit_2)>64)||((big_digit_1*10+big_digit_2)==0)){
					page=0;
					big_digit_1=10;
					big_digit_2=10;
				}
				else{
					page=3;
				}
				
				break;
		}
		BUT_FIX=2;
		update=1;
	}
}
//page=3		Информация о замке
void EMZ_info(void){
	uint8_t EMZ_num=big_digit_1*10+big_digit_2-1;
	if(update){
		lcd_bufer[0]=sistem_mode;
		lcd_bufer[1]=EMZ.status[EMZ_num];
		lcd_bufer[2]=EMZ.cap[EMZ_num];
		lcd_bufer[3]=DPNK_BLOCK;
	  if(!duble_open){
		lcd_clean();
		if(sistem_mode==1)lcd_set_string("Режим 1",42,0);
		else lcd_set_string("Режим 2",42,0);
		if(big_digit_1<10)lcd_set_big_digit(big_digit_1,1);
		if(big_digit_2<10)lcd_set_big_digit(big_digit_2,2);
		//EMZ.status[EMZ_num]=BLOCK;
		if(EMZ.status[EMZ_num]==DISCONNECT){
			lcd_set_string("Отсутствует",30,5);
			lcd_set_string("*-Отмена",0,7);
		}
		else if(EMZ.status[EMZ_num]==BLOCK){
			lcd_set_string("Заблокирован",30,5);
			if(sistem_mode==1){
				if(EMZ.cap[EMZ_num]==FULL){
					if(PU_mode==DPNK)lcd_set_string("*-Отмена   #-Разблок.",0,7);
					else lcd_set_string("*-Отмена",0,7);
				}
				else lcd_set_string("*-Отмена   Идет заряд",0,7);
			}
			else{
				if(EMZ.cap[EMZ_num]==FULL){
					if(PU_mode==DPNK)lcd_set_string("*-Отмена",0,7);
					else lcd_set_string("*-Отмена   #-Разблок.",0,7);
				}
				else lcd_set_string("*-Отмена   Идет заряд",0,7);
			}
		}
		else if(EMZ.status[EMZ_num]==OPEN){
			lcd_set_string("Разблокирован",27,5);
			if(EMZ.cap[EMZ_num]==FULL)lcd_set_string("*-Отмена",0,7);
			else lcd_set_string("*-Отмена   Идет заряд",0,7);
		}
		else if(EMZ.status[EMZ_num]==ERROR){
			lcd_set_string("Нет связи",36,5);
			lcd_set_string("*-Отмена",0,7);
		}
	  }
	  else{
		  lcd_clean();
		  if(sistem_mode==1)lcd_set_string("Режим 1",42,0);
		  else lcd_set_string("Режим 2",42,0);
		  lcd_set_string("Закройте",39,2);
		  lcd_set_string("все замки!",33,4);
		  lcd_set_string("*-Отмена",0,7);
		  duble_open=0;
	  }
	  update=0;
	}
	if(lcd_bufer[0]!=sistem_mode)update=1;
	if(lcd_bufer[1]!=EMZ.status[EMZ_num])update=1;
	if(lcd_bufer[2]!=EMZ.cap[EMZ_num])update=1;
	if(lcd_bufer[3]!=DPNK_BLOCK)update=1;
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:page=0;big_digit_1=10;big_digit_2=10;break;
			case 11:
				if(EMZ.status[EMZ_num]==BLOCK){
					if(EMZ.cap[EMZ_num]==FULL){
						if(sistem_mode==1){
							if(PU_mode==DPNK){
								if(DPNK_BLOCK==OPEN){
									open_EMZ=EMZ_num;
									send_EMZ=1;
									DPNK_BLOCK=BLOCK;
								}
								else{
									duble_open=1;
								}
							}
						}
						else {
							if(PU_mode==DK){
								if(EMZ.status[EMZ_num]==BLOCK){
									EMZ.out[EMZ_num]=OPEN;
									DPNK_BLOCK=BLOCK;
								}
							}
						}
					}
				}
				break;
			default:page=1;big_digit_1=BUTTON;big_digit_2=10;BUT_FIX=2;update=1;break;
		}
		BUT_FIX=2;
		update=1;
	}
}
															//ввод пароля page=2
void Set_password(){
	if(update){
		lcd_clean();
		lcd_set_string("Введите пароль",21,2);
		for(uint8_t i=0;i<4;i++){
			if(bufer_password[i]>9)lcd_set_ch('*',42+(12*i),4);
			else lcd_set_ch(bufer_password[i]+0x30,42+(12*i),4);
		}
		//lcd_set_string("*-Отмена",0,7);
		//lcd_set_string("Заблокирован",27,6);
		lcd_set_string("*-Отмена       #-Ввод",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
				page=0;
				bufer_password[0]=10;
				bufer_password[1]=10;
				bufer_password[2]=10;
				bufer_password[3]=10;
				break;
			case 11:
				if(reset_password<10){
					if((bufer_password[0]==10)&&(bufer_password[1]==10)&&(bufer_password[2]==10)&&(bufer_password[3]==10)){
						if(PU_mode==DK)reset_password++;
					}
					else{
						if(PU_mode){
							if((bufer_password[0]==user_password[0])&&(bufer_password[1]==user_password[1])&&(bufer_password[2]==user_password[2])&&(bufer_password[3]==user_password[3])){
								page=6;
								password_error=0;
							}
							else{
								page=22;
								password_error++;
							}
						}
						else{
							if((bufer_password[0]==admin_password[0])&&(bufer_password[1]==admin_password[1])&&(bufer_password[2]==admin_password[2])&&(bufer_password[3]==admin_password[3])){
								page=4;
								password_error=0;
							}
							else{
								page=22;
								password_error++;
							}
						}
						bufer_password[0]=10;
						bufer_password[1]=10;
						bufer_password[2]=10;
						bufer_password[3]=10;
						reset_password=0;
					}
				}
				else{
					page=21;
					bufer_password[0]=10;
					bufer_password[1]=10;
					bufer_password[2]=10;
					bufer_password[3]=10;
					admin_password[0]=0;
					admin_password[1]=0;
					admin_password[2]=0;
					admin_password[3]=0;
					user_password[0]=0;
					user_password[1]=0;
					user_password[2]=0;
					user_password[3]=0;
					write_eeprom(0x50,0);
					write_eeprom(0x51,0);
					write_eeprom(0x52,0);
					write_eeprom(0x53,0);
					write_eeprom(0x60,0);
					write_eeprom(0x61,0);
					write_eeprom(0x62,0);
					write_eeprom(0x63,0);
					reset_password=0;
				}
				if(password_error==5)page=10;
				break;
			default:
				if(bufer_password[0]==10)bufer_password[0]=BUTTON;
				else if(bufer_password[1]==10)bufer_password[1]=BUTTON;
				else if(bufer_password[2]==10)bufer_password[2]=BUTTON;
				else if(bufer_password[3]==10)bufer_password[3]=BUTTON;
				break;
		}
		BUT_FIX=2;
		update=1;
	}
	
}
							//переключение режима работы системы page=6
void Set_sistem_mode(){
	if(update){
		lcd_clean();
		if(sistem_mode==1){
			lcd_set_string("Режим 1",42,0);
			lcd_set_string("Включить",39,2);
			lcd_set_string("режим 2?",39,4);
		}
		else{
			lcd_set_string("Режим 2",42,0);
			lcd_set_string("Включить",39,2);
			lcd_set_string("режим 1?",39,4);
		}
		lcd_set_string("*-Нет            #-Да",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		if(BUTTON==10){
			page=0;
			update=1;
		}
		else if(BUTTON==11){
			page=0;
			if(sistem_mode==1)sistem_mode=2;
			else if(sistem_mode==2)sistem_mode=1;
			update=1;
		}
		BUT_FIX=2;
	}
}

									//ошибка ввода пароля page=22

void Error_password(void){
	if(update){
		lcd_clean();
		lcd_set_string("Не верный",36,2);
		lcd_set_string("пароль",45,4);
		lcd_set_string("*-Отмена",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		if(BUTTON==10){
			page=0;
		}
		BUT_FIX=2;
		update=1;
	}
}
										//сброс пароля page=21

void Reset_pass(void){
	if(update){
		lcd_clean();
		lcd_set_string("Пароли сброшены",18,3);
		lcd_set_string("*-Отмена",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		if(BUTTON==10)page=0;
		BUT_FIX=2;
		update=1;
	}
}


										//настройки page=4
void Seting_menu(void){
	if(update){
		lcd_clean();
		lcd_set_string("1-Пароль админа",0,0);
		lcd_set_string("2-Пароль ДПНК",0,1);
		lcd_set_string("3-Настройка ЭМЗ",0,2);
		lcd_set_string("4-Адресация ЭМЗ",0,3);
		if(!sistem_mode)lcd_set_string("5-Пульт ДПНК откл.",0,4);
		else lcd_set_string("5-Пульт ДПНК подкл.",0,4);
		lcd_set_string("6-Время тревоги",0,5);
		switch(alarm_time){
			case 0:lcd_set_string("откл.",96,5);break;
			case 1:lcd_set_string("0,25c",96,5);break;
			case 2:lcd_set_string("0,5c",96,5);break;
			case 3:lcd_set_string("0,75c",96,5);break;
			case 4:lcd_set_string("1c",96,5);break;
			case 5:lcd_set_string("1,25c",96,5);break;
			case 6:lcd_set_string("1,5c",96,5);break;
			case 7:lcd_set_string("1,75c",96,5);break;
			case 8:lcd_set_string("2c",96,5);break;
			case 9:lcd_set_string("2,25 c",96,5);break;
			default:;
		}
		lcd_set_string("7-Сброс настроек",0,6);
		lcd_set_string("*-Отмена",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
				page=0;
				write_eeprom(0x70,alarm_time);
				write_eeprom(0x71,sistem_mode);
			break;
			case 1:page=41;break;
			case 2:page=42;break;
			case 3:page=43;break;
			case 4:page=44;break;
			case 5:
				if(sistem_mode)sistem_mode=0;
				else sistem_mode=1;
			break;
			case 6:
				if(alarm_time<9)alarm_time++;
				else alarm_time=0;
			break;
			case 7:page=46;lcd_bufer[0]=0;break;
			default:;
		}
		BUT_FIX=2;
		update=1;
	}
}


							//сброс настроек  page=46
void Reset_seting(){
	if(update){
		lcd_clean();
		if(lcd_bufer[0]==0){
			lcd_set_string("Вернуть",42,1);
			lcd_set_string("к заводским",30,3);
			lcd_set_string("настройкам?",30,5);
			lcd_set_string("*-Нет            #-Да",0,7);
		}
		else{
			lcd_set_string("Настройки",36,1);
			lcd_set_string("восстановлены",24,3);
			lcd_set_string("до зоводских",27,5);
			lcd_set_string("*-Отмена",0,7);
		}
		update=0;
	}
	if(BUT_FIX==1){
		if(BUTTON==10){
			page=4;
			update=1;
		}
		else if(BUTTON==11){
			if(lcd_bufer[0]==0){
				for(uint8_t i=0;i<64;i++){
					EMZ.status[i]=BLOCK;
					write_eeprom(i,BLOCK);
					EMZ.position[i]=i;
					write_eeprom(i+0xC0,i);
					EMZ.adress[i]=i;
					write_eeprom(i+0x80,i);
				}
				write_eeprom(0x50,0);
				write_eeprom(0x51,0);
				write_eeprom(0x52,0);
				write_eeprom(0x53,0);
				admin_password[0]=0;
				admin_password[1]=0;
				admin_password[2]=0;
				admin_password[3]=0;
				
				write_eeprom(0x60,0);
				write_eeprom(0x61,0);
				write_eeprom(0x62,0);
				write_eeprom(0x63,0);
				user_password[0]=0;
				user_password[1]=0;
				user_password[2]=0;
				user_password[3]=0;
				
				write_eeprom(0x70,0);
				alarm_time=0;
				write_eeprom(0x71,1);
				sistem_mode=1;
				
				lcd_bufer[0]=1;
			}
			update=1;
		}
		BUT_FIX=2;
	}
}

								//пульт ДПНК отключен page=7

void DPNK_DISCONNECT(void){
	if(update){
		lcd_clean();
		lcd_set_string("Пульт ДПНК",33,3);
		lcd_set_string("отключен",39,5);
		update=0;
	}
	if(sistem_mode){
		page=0;
		update=1;
	}
	if(BUT_FIX==1){
		BUT_FIX=2;
		update=1;
	}
}

void Set_password_admin(void){
	if(update){
		lcd_clean();
		lcd_set_string("Введите пароль",21,2);
		lcd_set_string("администратора",21,3);
		for(uint8_t i=0;i<4;i++){
			if(bufer_password[i]>9)lcd_set_ch('*',42+(12*i),5);
			else lcd_set_ch(bufer_password[i]+0x30,42+(12*i),5);
		}
		lcd_set_string("*-Отмена",0,7);
		if(bufer_password[3]<10)lcd_set_string("#-ввод",90,7);
		//lcd_set_string("Заблокирован",27,6);
		//lcd_set_string("*-отмена       #-ввод",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
				page=4;
				bufer_password[0]=10;
				bufer_password[1]=10;
				bufer_password[2]=10;
				bufer_password[3]=10;
			break;
			case 11:
				if(bufer_password[3]<10){
					page=4;
					admin_password[0]=bufer_password[0];
					admin_password[1]=bufer_password[1];
					admin_password[2]=bufer_password[2];
					admin_password[3]=bufer_password[3];
					write_eeprom(0x50,admin_password[0]);
					write_eeprom(0x51,admin_password[1]);
					write_eeprom(0x52,admin_password[2]);
					write_eeprom(0x53,admin_password[3]);
					bufer_password[0]=10;
					bufer_password[1]=10;
					bufer_password[2]=10;
					bufer_password[3]=10;
				}
			break;
			default:
				if(bufer_password[0]==10)bufer_password[0]=BUTTON;
				else if(bufer_password[1]==10)bufer_password[1]=BUTTON;
				else if(bufer_password[2]==10)bufer_password[2]=BUTTON;
				else if(bufer_password[3]==10)bufer_password[3]=BUTTON;
			break;
		}
		BUT_FIX=2;
		update=1;
	}
}

void Set_password_user(void){
	if(update){
		lcd_clean();
		lcd_set_string("Введите пароль",21,2);
		lcd_set_string("ДПНК",51,3);
		for(uint8_t i=0;i<4;i++){
			if(bufer_password[i]>9)lcd_set_ch('*',42+(12*i),5);
			else lcd_set_ch(bufer_password[i]+0x30,42+(12*i),5);
		}
		lcd_set_string("*-Отмена",0,7);
		if(bufer_password[3]<10)lcd_set_string("#-ввод",90,7);
		//lcd_set_string("Заблокирован",27,6);
		//lcd_set_string("*-отмена       #-ввод",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
			page=4;
			bufer_password[0]=10;
			bufer_password[1]=10;
			bufer_password[2]=10;
			bufer_password[3]=10;
			break;
			case 11:
			if(bufer_password[3]<10){
				page=4;
				user_password[0]=bufer_password[0];
				user_password[1]=bufer_password[1];
				user_password[2]=bufer_password[2];
				user_password[3]=bufer_password[3];
				write_eeprom(0x60,user_password[0]);
				write_eeprom(0x61,user_password[1]);
				write_eeprom(0x62,user_password[2]);
				write_eeprom(0x63,user_password[3]);
				bufer_password[0]=10;
				bufer_password[1]=10;
				bufer_password[2]=10;
				bufer_password[3]=10;
			}
			break;
			default:
			if(bufer_password[0]==10)bufer_password[0]=BUTTON;
			else if(bufer_password[1]==10)bufer_password[1]=BUTTON;
			else if(bufer_password[2]==10)bufer_password[2]=BUTTON;
			else if(bufer_password[3]==10)bufer_password[3]=BUTTON;
			break;
		}
		BUT_FIX=2;
		update=1;
	}
}

void Activate_EMZ(void){
	uint8_t EMZ_num=big_digit_1*10+big_digit_2-1;
	if(update){
		lcd_clean();
		lcd_set_string("Номер ЭМЗ",36,0);
		lcd_set_string("*-Отмена",0,7);
		if(big_digit_1<10)lcd_set_big_digit(big_digit_1,1);
		if(big_digit_2<10){
			lcd_set_big_digit(big_digit_2,2);
			if(EMZ.status[EMZ_num]==DISCONNECT){
				lcd_set_string("Отключен",39,5);
				lcd_set_string("#-Включить",66,7);
			}
			else{
				lcd_set_string("Включен",42,5);
				lcd_set_string("#-Отключить",60,7);
			}
		}
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
			page=4;
			big_digit_1=10;
			big_digit_2=10;
			break;
			case 11:
			if(big_digit_2<10){
				if(EMZ.status[EMZ_num]==DISCONNECT){
					EMZ.status[EMZ_num]=BLOCK;
					write_eeprom(EMZ_num,BLOCK);
				}
				else {
					EMZ.status[EMZ_num]=DISCONNECT;
					write_eeprom(EMZ_num,DISCONNECT);
				}
			}
			break;
			default:
			if(big_digit_2<10)big_digit_2=10;
			if(big_digit_2==10){
				big_digit_1=BUTTON;
				big_digit_2=11;
			}
			else{
				big_digit_2=BUTTON;
				if((big_digit_1*10+big_digit_2>64)||(big_digit_1*10+big_digit_2==0)){
					big_digit_1=10;
					big_digit_2=10;
				}
			}
			break;
		}
		BUT_FIX=2;
		update=1;
	}
}

void Adresing_EMZ(void){
	uint8_t EMZ_num=big_digit_1*10+big_digit_2-1;
	if(update){
		lcd_clean();
		lcd_set_string("Адресация ЭМЗ",24,0);
		lcd_set_string("*-Отмена",0,7);
		lcd_set_string("> Номер ЭМЗ:",0,2);
		lcd_set_string("  Адрес ЭМЗ:",0,4);
		if(big_digit_1<10)lcd_set_ch(big_digit_1+0x30,80,2);
		if(big_digit_2<10){
			lcd_set_ch(big_digit_2+0x30,86,2);
			lcd_set_ch((EMZ.position[EMZ_num]+1)/10+0x30,80,4);
			lcd_set_ch((EMZ.position[EMZ_num]+1)%10+0x30,86,4);
			lcd_set_string("#-Изменить",68,7);
		}
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
			page=4;
			big_digit_1=10;
			big_digit_2=10;
			break;
			case 11:
			if(big_digit_2<10){
				page=45;
				lcd_bufer[0]=EMZ_num;
				big_digit_1=10;
				big_digit_2=10;
			}
			break;
			default:
			if(big_digit_2<10)big_digit_2=10;
			if(big_digit_2==10){
				big_digit_1=BUTTON;
				big_digit_2=11;
			}
			else{
				big_digit_2=BUTTON;
				if((big_digit_1*10+big_digit_2>64)||(big_digit_1*10+big_digit_2==0)){
					big_digit_1=10;
					big_digit_2=10;
				}
			}
			break;
		}
		BUT_FIX=2;
		update=1;
	}
}

void Adresing_EMZ_2(void){
	uint8_t EMZ_num=lcd_bufer[0];
	uint8_t EMZ_adress=EMZ.position[EMZ_num];
	if(update){
		lcd_clean();
		lcd_set_string("Адресация ЭМЗ",24,0);
		lcd_set_string("*-Отмена",0,7);
		lcd_set_string("  Номер ЭМЗ:",0,2);
		lcd_set_ch((EMZ_num+1)/10+0x30,80,2);
		lcd_set_ch((EMZ_num+1)%10+0x30,86,2);
		lcd_set_string("> Адрес ЭМЗ:",0,4);
		if(big_digit_1<10)lcd_set_ch(big_digit_1+0x30,80,4);
		if(big_digit_2<10){
			lcd_set_ch(big_digit_2+0x30,86,4);
			lcd_set_string("#-Изменить",68,7);
		}
		update=0;
	}
	if(BUT_FIX==1){
		switch(BUTTON){
			case 10:
			page=44;
			big_digit_1=(EMZ_num+1)/10;
			big_digit_2=(EMZ_num+1)%10;
			break;
			case 11:
			if(big_digit_2<10){
				page=44;
				EMZ_adress=(big_digit_1*10)+big_digit_2-1;
				uint8_t a=EMZ.position[EMZ_num];
				EMZ.position[EMZ_num]=EMZ_adress;
				uint8_t b=EMZ.adress[EMZ_adress];
				EMZ.adress[EMZ_adress]=EMZ_num;
				EMZ.adress[a]=b;
				EMZ.position[b]=a;
				write_eeprom(0x80+EMZ_adress,EMZ_num);
				write_eeprom(0xC0+EMZ_num,EMZ_adress);
				write_eeprom(0x80+a,b);
				write_eeprom(0xC0+b,a);
				big_digit_1=(EMZ_num+1)/10;
				big_digit_2=(EMZ_num+1)%10;
				
			}
			break;
			default:
			if(big_digit_2<10)big_digit_2=10;
			if(big_digit_2==10){
				big_digit_1=BUTTON;
				big_digit_2=11;
			}
			else{
				big_digit_2=BUTTON;
				if((big_digit_1*10+big_digit_2>64)||(big_digit_1*10+big_digit_2==0)){
					big_digit_1=10;
					big_digit_2=10;
				}
			}
			break;
		}
		BUT_FIX=2;
		update=1;
	}
}


/*

void Scan_EMZ(void){
	if(update){
		lcd_clean();
		if(!scan_index){
			lcd_set_string("Сканирование",27,2);
			WriteXY(14,4,1);
			WriteXY(0,4,2);
			scan_on=1;
		}
		else{
			for(uint8_t i=0;i<64;i++){
				if(EMZ.status[i]!=BLOCK){
					EMZ.status[i]=DISCONNECT;
					write_eeprom(i,DISCONNECT);
				}
			}
			lcd_set_string("Сканирование",27,2);
			lcd_set_string("завершено",36,4);
			lcd_set_string("*-Отмена",0,7);
		}
		update=0;
	}
	if(!timer_DPNK){
		if(scan_on==1){
			if(scan_index<50)WriteData(255,1);
			else if(scan_index<100)WriteData(255,2);
			else{
				scan_on=0;
				update=1;
			}
			scan_index++;
			timer_DPNK=1;
			TCNT1=40000;
			TIMSK1=ON;
		}
	}
	if(BUT_FIX==1){
		if(BUTTON==10){
			page=4;
			update=1;
		}
		BUT_FIX=2;
	}
}*/

void Error_wireles(void){
	if(update){
		lcd_clean();
		lcd_set_string("Нет связи",36,2);
		lcd_set_string("с пультом",36,4);
		if(PU_mode==DPNK)lcd_set_string("ДК",57,6);
		else lcd_set_string("ДПНК",51,6);
		//lcd_set_string("Заблокирован",27,6);
		//lcd_set_string("*-отмена       #-ввод",0,7);
		update=0;
	}
	if(BUT_FIX==1){
		if(PU_mode==DK){
			if(BUTTON==10)page=2;
		}
		BUT_FIX=2;
		update=1;
	}
}