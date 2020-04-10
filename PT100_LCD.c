#define F_CPU 1000000UL			/* Define CPU Frequency e.g. here 8MHz */
#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>			/* Include inbuilt defined Delay header file */
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#define Vref 5.07 

#define LCD_Data_Dir DDRD		/* Define LCD data port direction */
#define LCD_Command_Dir DDRB		/* Define LCD command port direction register */
#define LCD_Data_Port PORTD		/* Define LCD data port */
#define LCD_Command_Port PORTB		/* Define LCD data port */

#define RS PORTB2				/* Define Register Select (data/command reg.)pin */
#define RW PORTB1				/* Define Read/Write signal pin */
#define EN PORTB0				/* Define Enable signal pin */

volatile int Ain, AinLow ; 		// A/D read register  
 float voltage ;		//scaled input voltage
 float Resistance  = 0 ; 
 float temperature = 0 ;
 float calibration = 0 ;
 char v_string[10]; // scaled input voltage string to print

ISR (ADC_vect){

//program ONLY gets here when ADC done flag is set
//when reading 10-bit values
//you MUST read the low byte first
AinLow = ADCL;
Ain = ADCH*256;
Ain = Ain + AinLow;



}


void ADC_config() {
	
	ADMUX |= (1<<REFS0) ; // internal 2.54 |(1<<REFS1)
	ADCSRA |= (1<<ADEN) | (1<<ADIE)  ;
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescaler to 128 - 125KHz sample rate @ 16MHz
    ADCSRA |= (1 << ADATE);  // Set ADC to Free-Running Mode 
	ADCSRA |= (1 << ADSC);  // Start A2D Conversions 
	
}

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_ms(10);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(10);

}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;		/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;		/* Make LCD data port direction as o/p */
	_delay_ms(100);			/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);		/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);		/* Display ON Cursor OFF */
	LCD_Command (0x06);		/* Auto Increment cursor */
	LCD_Command (0x01);		/* Clear display */
	LCD_Command (0x80);		/* Cursor at home position */
}

void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)/* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* clear display */
	LCD_Command (0x80);		/* cursor at home position */
}

int main()
{
    ADC_config() ; 
	LCD_Init();			/* Initialize LCD */
	sei() ; 
	while(1) {
	
	voltage = Ain ;
	voltage = (voltage/1024.0)*Vref ; //(fraction of full scale)*Vref
	Resistance = ( 0.977*voltage*1000 ) /(19.7-voltage) ; 
	temperature = ((Resistance/100)-1)/0.00385 ; 
	calibration = 0.3+(0.005*temperature) ; // tolerance for class B PT100
	temperature =temperature - calibration ;
	dtostrf(temperature, 6, 3, v_string);
	LCD_Command(0x80);		/* Go to 2nd line*/
	LCD_String("Hassan Al-King0");	/* write string on 1st line of LCD*/
	LCD_Command(0xC0);		/* Go to 2nd line*/
	LCD_String("Temp = ");	/* write string on 1st line of LCD*/
	LCD_String(v_string);	/* Write string on 2nd line*/
	_delay_ms(1000) ; 
	
	
	}
	return 0;
}
