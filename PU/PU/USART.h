/*
 * USART.h
 *
 * Created: 12.01.2019 23:21:09
 *  Author: Олег
 */
#ifndef F_CPU
#define F_CPU 16000000L
#endif
 

#define BAUD 57600
#define MYUBRR F_CPU/8/BAUD-1

void USART_init(void){
	UBRR0L = MYUBRR; //baud rate 57600
	UCSR0A = (1<<U2X0);
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); //8 bit, 1 stop bit
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

