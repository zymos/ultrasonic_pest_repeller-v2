

PIN PORT 
 1: PB0 (MOSI/DI/SDA/OC1A)
 2: PB1 (MISO/DO/OC1A)
 3: PB2 (SCK/SCL/OC1B)
 4: PB3 (OC1B)
 5: VCC
 6: GND
 7: PB4 (ADC7/XTAL1)
 8: PB5 (ADC8/XTAL2)
 9: PB6 (ADC9/INT0/T0)
10: PB7 (ADC10/RESET)

20: PA0 (ADC0)
19: PA1 (ADC1)
18: PA2 (ADC2)
17: PA3 (AREF)
16: GND
15: AVCC
14: PA4 (ADC3)
13: PA5 (ADC4)
12: PA6 (ADC5/AIN0)
11: PA7 (ADC6/AIN1)

ADMUX ADC Multiplexer Select Register
	(REFS1)(REFS0)(ADLAR)(MUX4)(MUX3)(MUX2)(MUX1)(MUX0)

	REFS1 REFS0
	0 0 AVCC
	0 1 AREF (PA3), Internal Vref turned off.
	1 0 Internal Voltage Reference (2.56 V), AREF pin (PA3) not connected.
	1 1 Internal Voltage Reference (2.56 V), AREF pin (PA3).

	ADLAR: ADC Left Adjust Result

	MUX4..MUX0: ADC channel

ADCSR ADC Control and Status Register
	(ADEN)(ADSC)(ADFR)(ADIF)(ADIE)(ADPS2)(ADPS1)(ADPS0)

	ADEN: ADC Enable
	ADSC: ADC Start Conversion
	ADFR: ADC Free Running Select
	ADIF: ADC Interrupt Flag
	ADIE: ADC Interrupt Enable	
	ADPS2..0: ADC Prescaler Select Bits

ADCH ADC Data Register High
ADCL ADC Data Register Low

PORTA Data Register, Port A
DDRA Data Direction Register, Port A
PINA Input Pins, Port A
PORTB Data Register, Port B
DDRB Data Direction Register, Port B
PINB Input Pins, Port B


LCD
	 1: Vss (Gnd)
	 2: Vdd (5V)
	 3: Vee (Contrast)
	 4: RS
	 5: R/W
	 6: E
	 7: D0   
	 8: D1
	 9: D2
	10: D3
	11: D5 (D0)
	12: D6 (D1)
	13: D7 (D2)
	14: D8 (D3)
	15: LED Vcc?
	16: LED Gnd?
