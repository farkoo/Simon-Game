#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define	LCD_DPRT  PORTB 		//LCD DATA PORT
#define	LCD_DDDR  DDRB 		//LCD DATA DDR
#define	LCD_DPIN  PINB 		//LCD DATA PIN
#define	LCD_CPRT  PORTA 	//LCD COMMANDS PORT
#define	LCD_CDDR  DDRA 		//LCD COMMANDS DDR
#define	LCD_CPIN  PINA 		//LCD COMMANDS PIN
#define	LCD_RS  7 			//LCD RS
#define	LCD_RW  6 			//LCD RW
#define	LCD_EN  5 			//LCD EN

//***************************************************

char number[100];
int counter = 1;
int chcount = 0;
volatile int flag = 0;
int timer = 7811;
unsigned char colloc, rowloc;

//***************************************************

/*void generate_rand_1(){
	TCNT0 = 0x00;
	TCCR0 = 0x02;
	for(int i=0 ; i<100 ; i++){
		number[i] = 1 << (TCNT0 % 4);
	}
}*/

//***************************************************

void generate_rand(){
	srand(time(NULL));
	for(int i=0 ; i<100 ; i++){
		number[i] = 1 << (rand() % 4);
	}
}
//***************************************************

void delay(int timer)
{
	TCNT1 = 0;
	OCR1A = timer;
	TCCR1A = 0x00;
	TCCR1B = 0x0D;
	while((TIFR&(1<<OCF1A))==0)
	{ }
	TCCR1B = 0;
	TIFR = 1<<OCF1A;
}

//***************************************************

void lcdCommand( unsigned char cmnd )
{
  LCD_DPRT = cmnd;			//send cmnd to data port
  LCD_CPRT &= ~ (1<<LCD_RS);	//RS = 0 for command
  LCD_CPRT &= ~ (1<<LCD_RW);	//RW = 0 for write 
  LCD_CPRT |= (1<<LCD_EN);		//EN = 1 for H-to-L pulse
  _delay_us(1);				//wait to make enable wide
  LCD_CPRT &= ~ (1<<LCD_EN);	//EN = 0 for H-to-L pulse
  _delay_us(100);				//wait to make enable wide
}

//*******************************************************
void lcdData( unsigned char data )
{
  LCD_DPRT = data;			//send data to data port
  LCD_CPRT |= (1<<LCD_RS);		//RS = 1 for data
  LCD_CPRT &= ~ (1<<LCD_RW);	//RW = 0 for write
  LCD_CPRT |= (1<<LCD_EN);		//EN = 1 for H-to-L pulse
  _delay_us(1);				//wait to make enable wide
  LCD_CPRT &= ~ (1<<LCD_EN);	//EN = 0 for H-to-L pulse
  _delay_us(100);				//wait to make enable wide
}

//*******************************************************
void lcd_init()
{
  LCD_DDDR = 0xFF;
  LCD_CDDR = 0xFF;
 
  LCD_CPRT &=~(1<<LCD_EN);		//LCD_EN = 0
//  LCD_CPRT |= (1<<7);		//LCD_EN = 0
  _delay_us(2000);			//wait for init.
  lcdCommand(0x38);			//init. LCD 2 line, 5?7 matrix
  lcdCommand(0x0E);			//display on, cursor on
  lcdCommand(0x01);			//clear LCD
  _delay_us(2000);			//wait
  lcdCommand(0x06);			//shift cursor right
}

//*******************************************************
void lcd_gotoxy(unsigned char x, unsigned char y)
{  
 unsigned char firstCharAdr[]={0x80,0xC0,0x94,0xD4};//table 12-5  
 lcdCommand(firstCharAdr[y-1] + x - 1);
 _delay_us(100);	
}

//*******************************************************
void lcd_print( char * str )
{
  unsigned char i = 0 ;
  while(str[i]!=0)
  {
    lcdData(str[i]);
    i++ ;
  }
}

//***************************************************
ISR (INT0_vect)
{	
	int temp;
	if(PIND & (1 << PIND4))
	{
		PORTC |= (1<<0);
		temp = 1;
	}
	else if(PIND & (1 << PIND5))
	{
		PORTC |= (1<<1);
		temp = 2;
	}
	else if(PIND & (1 << PIND6))
	{
		PORTC |= (1<<2);
		temp = 4;
	}
	else if(PIND & (1 << PIND7))
	{
		PORTC |= (1<<3);
		temp =8;
	}
	delay(7811);
	PORTC = 0x00;
	if(number[chcount] != temp)
	{
		lcd_init();
		lcd_print("Game over!");
		counter = 0;
		chcount = 0;
		timer = 7811 + 50;
		flag = 1;
		_delay_ms(1000);
		return ;
	}
	else
	{
		//lcd_init();
		//lcd_print("success!");
		//delay(7811);
		chcount++;	
	}
	if(chcount == counter)
	{
		lcd_init();
		lcd_print("CORRECT");
		delay(7811);
		chcount = 0;
		cli();
		flag = 1;
	}
	return;
}
int main(void)
{
	lcd_init();
	DDRC = 0xFF;
	DDRD = 0x00;
	PORTD = 1<<2;
	MCUCR = 0x03;
	GICR = (1<<INT0);
	generate_rand();
    while (counter <= 100) 
    {
		lcd_init();
		lcd_print("WATCH");
		lcdCommand(0xC0);
		char Score[50];
		sprintf(Score,"Your Score: %d", counter-1);
		lcd_print(Score);
		delay(7811);
		for(char i=0 ; i<counter ; i++){
			PORTC = number[i];
			delay(timer);
			PORTC = 0x00;
			delay(timer);
		}
		PORTC = 0x00;
		lcd_init();
		lcd_print("PLAY");
		lcdCommand(0xC0);
		char score[50];
		sprintf(score,"Your Score: %d", counter-1);
		lcd_print(score);
		sei();
		
		while(flag == 0);
		flag = 0;
		
		counter++;
		timer -= 50;
	}
	while(1);
}


