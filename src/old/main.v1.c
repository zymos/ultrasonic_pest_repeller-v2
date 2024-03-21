/*
 * TempSensor.c
 *
 * Created: 4/13/201
 *
 * ATTiny26
 
 
 */ 
#define		F_CPU	4000000
#define 	XTAL   4000000

#define 	LCD_PORT   PORTA
#define 	LCD_DATA0_PORT   LCD_PORT
#define 	LCD_DATA1_PORT   LCD_PORT
#define 	LCD_DATA2_PORT   LCD_PORT
#define 	LCD_DATA3_PORT   LCD_PORT
#define 	LCD_DATA0_PIN   0	//pin11
#define 	LCD_DATA1_PIN   1	//pin12
#define 	LCD_DATA2_PIN   2	//pin13
#define 	LCD_DATA3_PIN   3	//pin14
#define 	LCD_RS_PORT   LCD_PORT
#define 	LCD_RS_PIN   4		//pin4
#define 	LCD_RW_PORT   LCD_PORT
#define 	LCD_RW_PIN   5		//pin5
#define 	LCD_E_PORT   LCD_PORT 
#define 	LCD_E_PIN   6		// pn6



#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd.h"
#include "lcd.c"


uint8_t ReadADC(uint8_t ch)
{
   //Select ADC Channel ch must be 0-7
   /* ch=0b00001001; */
   /* ADMUX|=ch; */

   //Start Single conversion
   ADCSR|=(1<<ADSC);

   //Wait for conversion to complete
   while(!(ADCSR & (1<<ADIF)));

   //Clear ADIF by writing one to it
   ADCSR|=(1<<ADIF);

   uint16_t adc_results = ADCH<<8 | ADCL;
   return(adc_results);
}

int main(void)
{

	/* initialize display, cursor off */
	lcd_init(LCD_DISP_ON);
	
	DDRB |= (0 << 4); 
		// PB6/ADC9/pin9 input
    ADMUX |= (1 << REFS1)|(0 << REFS0); 
		// Int Ref, pin not connected
	ADMUX |= (0 << ADLAR); 
		// Left Adjust
	ADMUX |= (0 << MUX4)|(1 << MUX3)|(0 << MUX2)|(0 << MUX1)|(1 << MUX0); 
		//ADC9

	ADCSR |= (1 << ADPS2)|(1 << ADPS2)|(1 << ADPS2); 
		// 128 divisor
	ADCSR |= (1 << ADEN); 
		// ADC enable
	ADCSR |= (1 << ADSC)|(1 << ADFR);
		// Start ADC in freerunning mode

	while(1)
    {
		/* clear display and home cursor */
		lcd_clrscr();
		     
		/* put string to display (line 1) with linefeed */
	
		uint16_t adc_result = ReadADC(9);
		uint16_t voltage_mv_b = adc_result*27 - 1024; // Read Analog value from channel-0		
		uint16_t temp_C = voltage_mv_b * 10 / 1024;

		uint16_t temp_F = ( 18 * temp_C + 320 ) / 10;

		/* uint16_t temp_dmC = voltage_dmv - 1000; */

		/* uint16_t temp_F = ( 1.8 * temp_dmC + 320000 ) / 10000; */

		/* uint32_t output_val = adc_result;		 */
		uint16_t output_val = temp_C;
		uint16_t output_val2 = temp_F;

		/* uint16_t adc_result = temp_dmC; */
		/* uint16_t adc_result = temp_F; */

		/* adc_result=ReadADC(9);  */
		/* adc_result=544; */
		/* 5/2 = 2; 5%2*10 = 5; result 2.5 */
		/* f = 1.8 * c + 32 */

		char line1result[10];		
		char line2result[10];		
		itoa(output_val, line1result, 10);
		itoa(output_val2, line2result, 10);

		lcd_puts("ADC = ");
		lcd_puts(line1result);
		lcd_puts("\n");
		/* cursor is now on second line, write second line */
		lcd_puts("ADC = ");
		lcd_puts(line2result);
		/* move cursor to position 8 on line 2 */
		/* lcd_gotoxy(1,2); */
		_delay_ms(500);
		/* write single char to display */
		/* lcd_putc(':'); */
		     
        //TODO:: Please write your application code 
    }
}
