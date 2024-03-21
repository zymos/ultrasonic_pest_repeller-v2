/*
 * Animal Deturent Ultrasonic Beeper and Flasher
 *
 *    Description: really annoy (hopfully) for pests,
 *       random ultrasonic beeps, 2-tone, and flashes
 *       Code is for Attiny26, but should be easly modified to any AVR
 *    
 *    Credits: Zef the Tinker, 2023/11
 *
 */ 

// define the frequency
#define	F_CPU	1000000
#define 	XTAL   1000000
#define __AVR_ATtiny26__ 1 // to use other MCU you need to change this

// #include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>



/////////////////////////////////////////////////////////////////////////////
// Config
//

#define TIMED_BEEP_INTERVAL 10 // Timed auto-activate interval (minutes)

// Deterrent action is (20 beeps * NUM_OF_LOOPS) + (4 * LED_BLINK_CNT)
#define NUM_OF_LOOPS 5 // number of times to loop through beeps (20 beeps * NUM_OF_LOOPS)
#define LED_BLINK_CNT 5 // number of times led will blink

// DEBUGING:  Normal opperation is 0/0/0 
// #define DISABLE_IRQ 0 // disable interrupt for testing
#define ALL_ON 0 // LED and SPEAKER always on, no blinks
// #define CONSTANT_BEEPING 1 // ALWAYS beeps and flashes, no motion sense


/////////////////////////////////////////////////////////////////////////////
// PINS
//
//    if you use other MCU you need to change this
//
// Interrupts
//    - PC interrupt1: PB[7:4], PA[7:6], PA[3]
//    - PC interrupt0: PB[3:0]
//    - interrupt0: PB[6]
//
// Pinout
// 1 PB0 (MOSI/DI/SDA/OC1A/PCINT0/PROGRAMMER) [PROGRAMER]
// 2 PB1 (MISO/DO/OC1A/PCINT0/PROGRAMMER)     [PROGRAMER]
// 3 PB2 (SCK/SCL/OC1B/PCINT0/PROGRAMMER)     [PROGRAMER]
// 4 PB3 (OC1B/PCINT0)                        [X]
// 5 VCC
// 6 GND
// 7 PB4 (ADC7/XTAL1/PCINT1)             [X]
// 8 PB5 (ADC8/XTAL2/PCINT1)             [X]
// 9 PB6 (ADC9/INT0/T0/PCINT1)           [PIR1]
// 10 PB7 (ADC10/RESET/PCINT1/PROGRAMER) [PROGRAMER]

// 20 PA0 (ADC0)                        [MODE SWITCH] 
// 19 PA1 (ADC1)                        [X]
// 18 PA2 (ADC2)                        [X]  
// 17 PA3 (AREF/PCINT1)                 [PIR0 SENSOR]  
// 16 GND
// 15 AVCC
// 14 PA4 (ADC3)                        [MCU ACTIVE LED]
// 13 PA5 (ADC4)                        [LED FLASH]
// 12 PA6 (ADC5/AIN0/PCINT1)            [SPEAKER0]
// 11 PA7 (ADC6/AIN1/PCINT1)            [SPEAKER1]
//
// notes: Vih-min for pins 7,8 are 0.8Vcc, 
//   all others are 0.6Vcc
//   So at 5V pins 7,8 need to be >4.0V
//
#define PIN_SPEAK0 PA6 // ultra-sonic speaker0, output
#define PIN_SPEAK1 PA7 // ultra-sonic speaker1, output

#define PIN_LED0 PB4 // flashing LED, output, drives 40mA
#define PIN_LED1 PB5 // flashing LED, output, drives 40mA

// #define PIN_LED_PWR_ON PA4 // LED to indicate MCU is up and running, output

// #define PIN_MODE_SW PA0 // Mode switch

#define PIN_PIRA PA3 // PIR
#define PIN_PIRB PB6 // PIR






//////////////////////////////////////////////////////////////////////////////
// Global vars
//

long time_counter = 0; // for timed auto activate interval

// #define MINUTE_FUDGE_FACTOR 1357000 //60 * 226000 ish

#define sleep_delay_multi 500 // multiplier for delay between auto-activate

#define LOOP_DELAY 100 // microseconds

#define delay_multi 15 // multiplier for the 1ms delay

// RANDOM beep durrations (psuedo-random)
const int beep_length[20] = {11,72,38,42,65,22,47,88,22,56
                              ,9,55,12,77,41,99,88,10,30,50 }; // list of random duration


// int speak0 = 0; // speaker 0 is on
// int speak1 = 0; // speaker 1 is on

//////////////////////////////////////////////////////////////////////////////
// Code
//


// make sure the freq isnt redefine, not sure if needed
#ifdef F_CPU
   #undef F_CPU
#endif
#define F_CPU	1000000

#ifdef XTAL
   #undef XTAL
#endif
#define XTAL   1000000




//////////////////////////////
// Primatives
//

void pwm_freq_change(int);
void animal_det(void);
void initialize(void);




//////////////////////////////
// Animal Deturent function
//    beeps and flashes, 
//    psuedo random durration and pauses
//    
void animal_det(void){
   // int pointer1 = 2; // used for speaker1 as random num list offset

   // repeat loops of array
   for(int q=0; q<NUM_OF_LOOPS; q++){
      // loop the psuedo-radom array
      //  this beep for each number in array

      // change the PWM frequency
      pwm_freq_change(q);

      // turn on/off PWM to make beeps
      for(int x=0; x < sizeof(beep_length)/sizeof(beep_length[0]); x++){ //loop the array
     
         // Speakers
         PORTA &= ~(1 << PIN_SPEAK0); //speaker 0 off
         // PORTA &= ~(1 << PIN_SPEAK1); //speaker 1 off
         TCCR1A &= ~(1<<COM1B1); // PWM off
         // speak0 = 0;
         // speak1 = 0;
         for(int n=0; n<100; n++){ //on/off period
            // if(pointer1 == 10){ // for ponter1 offset
               // pointer1 = 0;
            // }
            if(n > beep_length[x]){
               PORTA |= (1 << PIN_SPEAK0); //speaker 0 on
               TCCR1A |= (1<<COM1B1); // PWM on
               // speak0 = 1;
            }
            // if(n > beep_length[pointer1]){
               // PORTA |= (1 << PIN_SPEAK1); //speaker 1 on
               // speak1 = 1;
            // }
            _delay_ms(3 * delay_multi/10); 
         }   

         //LED
         if(x % 2 == 0){ // activate LED every N beeps
            for(int p=0; p<LED_BLINK_CNT; p++){
               PORTB |= (1 << PIN_LED0); //led on
               PORTB |= (1 << PIN_LED1); //led on
               _delay_ms(1 * delay_multi); 
               PORTB &= ~(1 << PIN_LED0); //led off
               PORTB &= ~(1 << PIN_LED1); //led off
               _delay_ms(1 * delay_multi);
            }
         }

         // inc pointer for speaker1
         // pointer1++;
      }
      
      // turn everything off
      PORTB &= ~(1 << PIN_LED0); //led 0 off
      PORTA &= ~(1 << PIN_SPEAK0); //speaker 0 off
      TCCR1A &= ~(1<<COM1B1); // PWM off
      // PORTA &= ~(1 << PIN_SPEAK1); //speaker 1 off
      // speak0 = 0;
      // speak1 = 0;

      _delay_ms(50 * delay_multi);
   }
}


////////////////////////////
// Frequency changing
//   changing PWM frequency to psuedo random list
//
void pwm_freq_change(int mode){
   if(mode == 0){
      /* 30kHz - 0101 132; */
      TCCR1B |= (1<<CS10);
      OCR1C = 132;
      OCR1B = 50; //65;
   }else if (mode == 1)
   {
      /* 40kHz - 0100 199 */
      TCCR1B &= ~(1<<CS10);
      OCR1C = 199;   
      OCR1B = 80; //100;
   }else if (mode == 2)
   {
      /* 37kHz - 0101 110; */
      TCCR1B |= (1<<CS10);
      OCR1C = 110;
      OCR1B = 45;//; //55;
   }else if (mode == 3)
   {
      /* 50kHz - 0100 159 */
      TCCR1B &= ~(1<<CS10);
      OCR1C = 159;   
      OCR1B = 70; //80;
   }else if (mode == 4)
   {
      /* 39kHz - 0101 110; */
      TCCR1B |= (1<<CS10);
      OCR1C = 120;
      OCR1B = 45; //55;
   }else{
      /* shouldn't happen but... ~30kHz - 0101 132; */
      TCCR1B |= (1<<CS10);
      OCR1C = 120;
      OCR1B = 50; //60;
   }
} 


////////////////////////////
// Interrupt on INT0
//  to use other MCU you need to change this
//    INT0_vect: interrupt 0
//    IO_PINS_vect: pin change interrupt (all?)

// ISR(IO_PINS_vect){ // to use other MCU you need to change this
//    //   See: https://www.gnu.org/savannah-checkouts/non-gnu/avr-libc/user-manual/group__avr__interrupts.html
//
//    // if interrupt and one of PIRs active
//    if( (PINB & (1 << PIN_PIRB) || PINA & (1 << PIN_PIRA) ) && !DISABLE_IRQ && !ALL_ON){
//
//       // (PINA & (1 << PIN_MODE_SW)){{
//
//          // activate deturrent
//          animal_det();
//       //}}
//
//    }
// }

// ISR (TIMER1_OVF1_vect)
// { 
//    // note vector name is defined in avr/include/avr/iotn26.h
// /* Toggle a pin on timer overflow */
//    TCNT1 = 252;
//    if(speak0){
//       PORTA ^= (1<<PIN_LED_PWR_ON); 
//    }else{
//             PORTA &= ~(1<<PIN_LED_PWR_ON); 
//    }
// }

/////////////////
// Initalize
void initialize(void){
   // set PORTA to all outputs, except PA3
   // PORTB is already inputs
   // to use other MCU you need to change this
   DDRA = 0b11110110; // set PORTA to all outputs, except PA3, PA0 // to use other MCU you need to change this

   // Set LED as output
   DDRB |= (1<<PIN_LED0);
   DDRB |= (1<<PIN_LED1);
   DDRB |= (1<<PB3);
	
   // ATtiny26
   //    MCUCR: MCU Control Register
   //       ISC01=1 ISC00=1 The rising edge of INT0 generates an interrupt request.
   //    GIMSK: General Interrupt Mask Register
   //       INT0: interrupt0
   //       PCIE1: pin change interrupt 1  PB[7:4], PA[7:6] and PA[3]
   //       PCIE0:  pin change interrupt 0 PB[3:0]
   //    GIFR: General Interrupt Flag Register
   //       GIFR(PCIF) flag when PC interrupt
   //    INTF0: 6
 // Using PB4 and PB5 for PCINT1
   // MCUCR |= (1<<ISC00); //rising edge // to use other MCU you need to change this
   // MCUCR |= (1<<ISC01); //rising edge // to use other MCU you need to change this
   // GIMSK |= (1<<INT0); // enable INT0 // to use other MCU you need to change this
   // GIMSK |= (1<<PCIE1); // enable PCINT1 // to use other MCU you need to change this

   // timer0
   // CS02=1, CS01=0, CS00=1: CLK/1024
   // TCCR0

   // /* Timer0  */
   // TCCR0 = (1<<CS00);
   // /* Clear overflow flag */
   // TIFR = 1<<TOV0;
   // /* Enable Overflow Interrupt */
   // TIMSK = 1<<TOIE0;

   // /* Timer1, asyncronous, fast periferial clk */
   // PLLCSR = (1<<PCKE); //FPK 64MHz
   // TCCR1B = (1<<CS10); //(1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10); // clk divider
   // TIMSK = (1<<TOIE1); // enable overflow interrupt

   PLLCSR = (1<<PCKE)|(1<<PLLE); //FPK/PLL on, 64MHz
   TCCR1A = (1<<COM1B1)|(1<<PWM1B); //PWM enabled
   TCCR1B = (1<<CS12)|(1<<CS10); //(1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10); // clk divider 20kHz
   OCR1C = 132; // 30kHz 
   OCR1B = 100; // 50% duty
   // COM1B1 = 0 to stop?

   // CS13..CS10 = 0101
   // OCR1C = 199
   // PCKE = 1 && PLLE = 1
   // FPK on
   // OC1B on
   // PWM1B: Pulse Width Modulator B Enabled

   sei();     // Enable global interrupts by setting global interrupt enable bit in SREG


   // turn on LED indicator to show MCU active
   // PORTA |= (1<<PIN_LED_PWR_ON); // to use other MCU you need to change this

}


///////////
// Main
int main(void)
{
   initialize();

	while(1) {
      if(ALL_ON){ // Mode: all lights and sound on (debug forced)
         PORTB |= (1 << PIN_LED0);
         PORTB |= (1 << PIN_LED1);
         PORTA |= (1 << PIN_SPEAK0);
         PORTA |= (1 << PIN_SPEAK1); 
      }else{ // Mode: Constant beeping mode (debug forced)
         animal_det();
      // }else if(PINA & (1 << PIN_MODE_SW)){ // Mode: (switch set)
      //    animal_det();
      // }
      }
      // else if( time_counter > TIMED_BEEP_INTERVAL * 60000 ){//TIMED_BEEP_INTERVAL * MINUTE_FUDGE_FACTOR / LOOP_DELAY){ // timed beeping
         // animal_det();
         // time_counter = 0; // reset counter
      // }
      _delay_ms(10000); //LOOP_DELAY);
      time_counter++;

   } // infinite loop


} //main
