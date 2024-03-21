/*
 * TempSensor.c
 *
 * Created: 4/13/201
 *
 * ATTiny26
 
 
 */ 
#define		F_CPU	1000000
#define 	XTAL   1000000

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
#define 	LCD_RS_PIN   4		//pin
#define 	LCD_RW_PORT   LCD_PORT
#define 	LCD_RW_PIN   5		//pin
#define 	LCD_E_PORT   LCD_PORT 
#define 	LCD_E_PIN   6		// pn



#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd.h"
#include "lcd.c"
/* #include <avr/interrupt.h> */
/* http://homepage.hispeed.ch/peterfleury/group__pfleury__lcd.html */



// Global vars
uint8_t env_mode;
uint8_t fan_status;
uint16_t indoor_temp;
uint16_t indoor_temp2;
uint16_t outdoor_temp;
uint8_t tictoc;
uint8_t stabilize_count;
uint8_t stabilize_delay;



void adc_init(void){
    ADMUX |= (1 << REFS1)|(0 << REFS0); 
		// Int Ref, pin not connected
	ADMUX |= (0 << ADLAR); 
		// Left Adjust
	/* ADMUX |= (0 << MUX4)|(1 << MUX3)|(0 << MUX2)|(0 << MUX1)|(1 << MUX0);  */
		//ADC9

	ADCSR |= (1 << ADPS2)|(1 << ADPS2)|(1 << ADPS2); 
		// 128 divisor
	ADCSR |= (1 << ADEN); 
		// ADC enable
	ADCSR |= (1 << ADSC)|(0 << ADFR);
		// Start ADC in freerunning mode
}


uint16_t adc_read(uint8_t ch)
{
   //Select ADC Channel ch must be 0-7
   /* ch=0b00001001; */
   	ADMUX &= 0xF0;
	ADMUX|=ch;

   //Start Single conversion
   ADCSR |= (1<<ADSC);

   //Wait for conversion to complete
   while(!(ADCSR & (1<<ADIF)));

   //Clear ADIF by writing one to it
   ADCSR |= (1<<ADIF);

   /* uint16_t adc_results = ADCH<<8 | ADCL; */

   return(ADC);
}

uint16_t adc_average(uint8_t adc_channel) {
	uint16_t result=0;
	uint16_t result2=0;

		uint8_t adc_samples=50;	//times 2
		uint8_t adc_delay=100; //us
		uint8_t z;	

		// average first half
		for(z=0;z<adc_samples;z++){
	 		result = result + adc_read(adc_channel); // Read Analog value from channel-0
			_delay_us(adc_delay);
		}
		result = result / adc_samples;

		// average second half
		for(z=0;z<adc_samples;z++){
	 		result2 = result2 + adc_read(adc_channel); // Read Analog value from channel-0
			_delay_us(adc_delay);
		}
		result2 = result2 / adc_samples;

		// average 2 halfs
		result = result2 + result;
		result =  result / 2;

		return(result);

}


uint16_t lm35_to_f(uint16_t adc_in)
{
	
	/* adc_in*(2.7V/1024b)*(1C/0.01V) = Temp C */
	/* adc_in*(270C/1024b) = Temp C */
	/* adc_in*(2700C/1024b) = 10x Temp C */
	/* adc_in*(27*100/32/32) = 10x Temp C */
	/* adc_in*(100/32)*(27/32) = 10x Temp C */

	uint16_t out = 100 * adc_in;
	out = out / 32;
	out = out * 27;
	out = out / 32;
	return out;


	/* old code */
	/* uint16_t voltage_mv_b = adc_in*27; // Read Analog value from channel-0		 */
	/* uint16_t temp_C = voltage_mv_b * 10 / 1024; */
	/* uint16_t temp_F = ( 18 * temp_C + 320 ); // / 10; */
    /* uint16_t temp_out = temp_C; */
	/* _delay_ms(10); // delay between samples */
	/* return(temp_out); */

		/* 5/2 = 2; 5%2*10 = 5; result 2.5 */
		/* f = 1.8 * c + 32 */
}

uint16_t lm35_to_f_twisted(uint16_t adc_in)
{
	uint8_t fudge_shift=16; //negative
	uint16_t out =  lm35_to_f(adc_in);
	out = out - 100 - fudge_shift;
	return out;
	
	/* old code */
	/* uint16_t voltage_mv_b = adc_in*27 - 1024; // Read Analog value from channel-0		 */
	/* uint16_t temp_C = voltage_mv_b * 10 / 1024; */
	/* uint16_t temp_F = ( 18 * temp_C + 320 ); // / 10; */
    /* uint16_t temp_out = temp_C; */
	/* _delay_ms(10); // delay between samples */
	/* return(temp_out); */

		/* 5/2 = 2; 5%2*10 = 5; result 2.5 */
		/* f = 1.8 * c + 32 */
}


void fan_control( void )
{
	/* bit 1: fan out 1 */
	/* bit 2: fan out 2 */
	/* bit 3: fan in */

	uint16_t trip_point_1 = 210;
	uint16_t trip_point_2 = 255;
	uint16_t trip_point_3 = 270;
	uint16_t trip_point_4 = 300;
	uint8_t hysteresis = 13;

	/* if( env_mode == 0 ){ */
	// mode: off
		/* fan_status |= (0 << 2)|(0 << 1)|(0 << 0);	 */
		/* fan_status = 0b00000000; */
	/* }else{ */


		if(indoor_temp <= trip_point_1 - hysteresis){
			// T < 67
			fan_status = 0b00000000;
		}else if(indoor_temp > trip_point_1 - hysteresis && indoor_temp <= trip_point_1){
			// 67 < T < 70 , hysteresis
			if(bit_is_set(fan_status, 0)){
				fan_status = 0b00000001; //fan 1 remain low, hysteresis
			}else{
				fan_status = 0; // all fans off
			}
		}else if(indoor_temp > trip_point_1 && indoor_temp < trip_point_2 - hysteresis){
			// 70 < T < 72
			fan_status = 0b00000001; // fan 1 low
		}else if(indoor_temp >= trip_point_2 - hysteresis && indoor_temp < trip_point_2){
			// 72 < T < 75, hysteresis
			if(bit_is_set(fan_status, 2)){
				fan_status = 0b00000010; //fan 1 remains high, hysteresis
			}else{
				fan_status = 0b00000001; // fan 2 off, fan 1 low
			}
		}else if(indoor_temp >= trip_point_2 && indoor_temp < trip_point_3 - hysteresis){
			// 75 < T < 77
			fan_status = 0b00000010; //fan 2 off, fan 1 high
		}else if(indoor_temp >= trip_point_3 - hysteresis && indoor_temp < trip_point_3){
			// 77 < T < 80, hysteresis
			if(bit_is_set(fan_status, 3)){
				fan_status = 0b00000110; //fan 2 remain on, hysteresis
			}else{
				fan_status = 0b00000010; // fan 3 off, fan 2 off, fan 1 high
			}
		}else if(indoor_temp >= trip_point_3 && indoor_temp < trip_point_4 - hysteresis){
			// 80 < T < 82
			fan_status = 0b00000110; //fan 3 off, fan 2 on, fan 1 high
		}else if(indoor_temp >= trip_point_4 - hysteresis && indoor_temp < trip_point_4){
			// 82 < T < 85, hysteresis
			if(bit_is_set(fan_status, 3)){
				fan_status = 0b00001110; // all on, hysteresis
			}else{
				fan_status = 0b00000110; // fan 3 off, fan 2 on, fan 1 high
			}
		}else if(indoor_temp >= trip_point_4){
			// T > 85
			fan_status = 0b00001110; // all on high
		}
	/* } */

	PORTB &= 0b11110000; //fixme
	PORTB |= fan_status;


	/* return fan_status; */

}



void lcd_output( void )
{
	lcd_clrscr();
	
	if( stabilize_count <= stabilize_delay ){ // seconds?
		lcd_puts("Stabilizing...");
		stabilize_count++;
	}else{

		char line1result[10];		
		char line2result[10];		
		itoa(indoor_temp, line1result, 10);
		itoa(outdoor_temp, line2result, 10);

		if( tictoc == 0 ){
			lcd_puts("I: ");
			tictoc++;
		}else{
			lcd_puts("I; ");
			tictoc = 0;
		}
		lcd_putc(line1result[0]);
		lcd_putc(line1result[1]);
		lcd_puts(".");	
		lcd_putc(line1result[2]);

		/* lcd_puts(line1result); */
		/* lcd_puts(" F"); */
		lcd_gotoxy(8, 0);
	
		lcd_puts("O: ");
		lcd_putc(line2result[0]);
		lcd_putc(line2result[1]);
		lcd_puts(".");	
		lcd_putc(line2result[2]);

		/* lcd_puts(" F"); */
		
		lcd_gotoxy(0, 1);
		lcd_puts("S: ");

		if( env_mode == 0 ){
			lcd_puts("off");
		}else if( env_mode == 1 ){
			lcd_puts("away");
		}else{
			lcd_puts("home");
		}


		lcd_gotoxy(8, 1);

		lcd_puts("Fan: ");
		if (fan_status & (1 << 0)){
			lcd_puts("+");
		}else if(fan_status & (1 << 1)){
			lcd_puts("#");
		}else{
			lcd_puts("-");
		}
		if (fan_status & (1 << 2)){
			lcd_puts("#");
		}else{
			lcd_puts("-");
		}
		if (fan_status & (1 << 3)){
			lcd_puts("#");
		}else{
			lcd_puts("-");
		}
	}
}



int main(void)
{
	stabilize_count=0;
	stabilize_delay=5;

	
	
	/* initialize display, cursor off */
	lcd_init(LCD_DISP_ON);
	adc_init();
	lcd_clrscr();

	DDRA &= ~(1 << DDA7);  // PD0 is now an input
	PORTA |= (1 << PORTA7);   // turn On the Pull-up enabled
	// port A7, input with pullup resistor


	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2)| (1 << DDB3);
	/* DDRA |= (1 << DDA7); */
	// Port B0, B1, B2, A7: fan control (outputs)
	
	while(1) {


		uint8_t x;		
		for ( x = 0; x < 60; x++ ) { //loop 60 sec
			
			
			// Check button and delay
			uint8_t y;
			for ( y = 0; y < 10; y++ ) { //loop 0.5 sec
				_delay_ms(50);
				if (bit_is_clear(PINA, PA7)) {
					_delay_ms(150); 
					env_mode++;
					if(env_mode == 3){
						env_mode = 0;
					}
					lcd_output(); // update mode
				} else {
				}
			} //loop 0.5s


			
			
			
			// Grab temp
			/* uint8_t adc_samples=50;	 */
			uint8_t adc_channel=9;
			/* uint8_t adc_delay=0; */
			/* uint8_t z;	 */
			/* indoor_temp = 0; */
			/* for(z=0;z<adc_samples;z++){ */
				 /* indoor_temp = indoor_temp + adc_read(adc_channel); // Read Analog value from channel-0 */
				/* _delay_ms(adc_delay); */
			/* } */
			/* indoor_temp = indoor_temp / adc_samples; */
			/* for(z=0;z<adc_samples;z++){ */
				 /* indoor_temp2 = indoor_temp2 + adc_read(adc_channel); // Read Analog value from channel-0 */
				/* _delay_ms(adc_delay); */
			/* } */
			/* indoor_temp2 = indoor_temp2 / adc_samples; */
			/* indoor_temp = indoor_temp2 + indoor_temp; */
			/* indoor_temp =  indoor_temp / 2; */
			
			indoor_temp = adc_average(adc_channel);
			indoor_temp = lm35_to_f( indoor_temp ); 

			
			adc_channel=8;
	
			/* for(z=0;z<adc_samples;z++){ */
				 /* outdoor_temp = outdoor_temp + adc_read(adc_channel); // Read Analog value from channel-0 */
				/* _delay_ms(adc_delay); */
			/* } */
			/* outdoor_temp = outdoor_temp / adc_samples; */
			outdoor_temp = adc_average(adc_channel);			
			outdoor_temp = lm35_to_f_twisted( outdoor_temp ); 
			
			
			
			
			
			
			/* outdoor_temp =indoor_temp ; */

				/* indoor_temp = lm35_to_f( (adc_read(9)+adc_read(9)+adc_read(9) + adc_read(9)+adc_read(9)+adc_read(9))/6 ); */
				/* outdoor_temp = lm35_to_f( (adc_read(8)+adc_read(8)+adc_read(8) + adc_read(8)+adc_read(8)+adc_read(8))/6 ); */


			// Display
			lcd_output();
		


		} //loop once a min

		/* _delay_ms(500);  */
		/* lcd_clrscr(); */
	  if( stabilize_count > stabilize_delay ){ // seconds?
			fan_control(); // update fan
	  }
		_delay_ms(500); 
		/* lcd_clrscr(); */


    } // infinite loop
} //main
