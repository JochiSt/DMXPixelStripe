/*----------------------------------------------------------------------------
 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
 GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
 (nach Ihrer Option) jeder späteren Version. 

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
 Programm erhalten haben. 
 Falls nicht, schreiben Sie an die Free Software Foundation, 
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.   
------------------------------------------------------------------------------*/
#define MAX_WS2811_LEDS 	30		// our 1m strip has 30 LEDs
#define DMX_BAUD 		250000
#define MAX_DMX_ADDR		512		// maximal dmx address

#include "global.h"
#include "config.h"

#include <util/delay.h>
#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "light_ws2812.h"
#include "timer.h"

#define DO_STARTUP		// enable startup lightshow
//#define DEBUG_SWITCHES	// Do not react on DMX instead display switch status

// FUSES:
// -U lfuse:w:0xf7:m -U hfuse:w:0xdf:m -U efuse:w:0x01:m

uint16_t dmx_adresse = 1;	// set to some default value -> will be overwritten

volatile uint8_t dmx_buffer_in[512];
volatile uint8_t dmx_valid = 0;
volatile uint8_t dmx_change = 0;

struct cRGB led_values[MAX_WS2811_LEDS];	//NOTE: GRB notation 
						// LED red   = struct green
						// LED green = struct red
uint8_t mode = 1;

void updateAddrMode(void);
void testSwitchMode(void);
//############################################################################
//DMX Empfangsroutine (RS485)
ISR (USART_RX_vect){
	static unsigned int dmx_channel_rx_count = 0;
	unsigned char tmp = 0;
	
	tmp =  UDR0;
	
	if(UCSR0A&(1<<FE0)){	
		dmx_change = 1;
		dmx_channel_rx_count = 0;
		dmx_buffer_in[0] = tmp;
		
		if(dmx_buffer_in[0] == 0){
			if(dmx_valid < 255){
				dmx_valid++;
			}
			dmx_channel_rx_count++;
		}else{
			dmx_valid = 0;
		}
		return;
	}
	
	if(dmx_valid > 1){
		dmx_buffer_in[dmx_channel_rx_count] = tmp;
				
/*
		if((dmx_channel_rx_count%3) == 0){
			tmp = dmx_buffer_in[dmx_channel_rx_count-2];
			dmx_buffer_in[dmx_channel_rx_count-2] = dmx_buffer_in[dmx_channel_rx_count-1];
			dmx_buffer_in[dmx_channel_rx_count-1] = tmp;
		}
*/
		
		if(dmx_channel_rx_count < 512){
			dmx_channel_rx_count++;
		}
		return;
	}
}

void updateAddrMode(){
	dmx_adresse  = 0;	
	dmx_adresse  =  !IS_SET(A0);
	dmx_adresse |= (!IS_SET(A1))<<1;
	dmx_adresse |= (!IS_SET(A2))<<2;
	dmx_adresse |= (!IS_SET(A3))<<3;
	dmx_adresse |= (!IS_SET(A4))<<4;
	dmx_adresse |= (!IS_SET(A5))<<5;
	dmx_adresse |= (!IS_SET(A6))<<6;
	dmx_adresse |= (!IS_SET(A7))<<7;
	dmx_adresse |= (!IS_SET(A8))<<8;

	if(dmx_adresse == 0) dmx_adresse = 1;

	mode  = 0;	
	mode  =  !IS_SET(M0);
	mode |= (!IS_SET(M1))<<1;
	mode |= (!IS_SET(M2))<<2;

	// TODO check that address is low enough
}

void testSwitchMode(){
	updateAddrMode();
	uint8_t local_cnt = 0;
	for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
		// 1st encode mode into color
		if(mode == 0){
			led_values[i].g = 255;	// red
			led_values[i].r = 255;	// green
			led_values[i].b = 255;	// blue
		}else{
			led_values[i].g = (mode & 1) ? 128 : 0;	// R
			led_values[i].r = (mode & 2) ? 128 : 0; // G
			led_values[i].b = (mode & 4) ? 128 : 0; // B
		}

		// 2nd encode address into led position
		if( dmx_adresse != 0 ){
			led_values[i].g = (dmx_adresse & (1 << local_cnt)) ? led_values[i].g : 0 ;
			led_values[i].r = (dmx_adresse & (1 << local_cnt)) ? led_values[i].r : 0 ;
			led_values[i].b = (dmx_adresse & (1 << local_cnt)) ? led_values[i].b : 0 ;
		}else{	// if address is zero reduce brightness by factor 2
			led_values[i].g = led_values[i].r >> 1;
			led_values[i].r = led_values[i].g >> 1;
			led_values[i].b = led_values[i].b >> 1;
		}

		// repeat address twice
		local_cnt++;
		if(local_cnt > 15){
			local_cnt = 0;
		}
	}
	ws2812_setleds(led_values,MAX_WS2811_LEDS);
}

//############################################################################
//Hauptprogramm
int main(void){
//############################################################################

	// Pin for direction
	// set to output and set it to zero
	DDRD |= (1<<PD2);
	PORTD &= ~(1<<PD2);

//#############################################################################

	// Set Input & enable PULLUP
	SET_INPUT(A0); SET(A0);
	SET_INPUT(A1); SET(A1);
	SET_INPUT(A2); SET(A2);
	SET_INPUT(A3); SET(A3);
	SET_INPUT(A4); SET(A4);
	SET_INPUT(A5); SET(A5);
	SET_INPUT(A6); SET(A6);
	SET_INPUT(A7); SET(A7);
	SET_INPUT(A8); SET(A8);

	SET_INPUT(M0); SET(M0);
	SET_INPUT(M1); SET(M1);
	SET_INPUT(M2); SET(M2);

//#############################################################################
#ifdef DO_STARTUP
	for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
		led_values[i].g = 255;	// red
		led_values[i].r = 0;	// green
		led_values[i].b = 0;	// blue
		ws2812_setleds(led_values,MAX_WS2811_LEDS);
		_delay_ms(50);
	}
	_delay_ms(250);
	for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
		led_values[i].g = 0;	// red
		led_values[i].r = 255;	// green
		led_values[i].b = 0;	// blue
		ws2812_setleds(led_values,MAX_WS2811_LEDS);
		_delay_ms(50);
	}
	_delay_ms(250);
	for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
		led_values[i].g = 0;	// red
		led_values[i].r = 0;	// green
		led_values[i].b = 255;	// blue
		ws2812_setleds(led_values,MAX_WS2811_LEDS);
		_delay_ms(50);
	}
	_delay_ms(250);
	for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
		led_values[i].g = 0;	// red
		led_values[i].r = 0;	// green
		led_values[i].b = 0;	// blue
		ws2812_setleds(led_values,MAX_WS2811_LEDS);
		_delay_ms(50);
	}
#endif
//#############################################################################

	//Init usart DMX-BUS
	UBRR0   = (F_CPU/ (DMX_BAUD * 16L) - 1);
	UCSR0B |=(1 << RXEN0 | 1<< RXCIE0);
	UCSR0C |=(1<<USBS0);			// USBS0 2 Stop bits

//#############################################################################
	// TEST Switch connection
#ifdef DEBUG_SWITCHES
	while(1){
		testSwitchMode();
	}
#endif
	testSwitchMode();
	_delay_ms(1000);
//#############################################################################
	
	timer_init();

	sei();//Globale Interrupts Enable	
			
//#############################################################################
//#############################################################################

	while(1){		
		if (dmx_change && (dmx_valid>2)){
			dmx_valid = 0;
			uint16_t local_dmx_addr = dmx_adresse;

			// generate LED colors from DMX values
			if(mode & 1){
				for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
					led_values[i].g = dmx_buffer_in[dmx_adresse];
					led_values[i].r = dmx_buffer_in[dmx_adresse+1];
					led_values[i].b = dmx_buffer_in[dmx_adresse+2];
				}
			}else if(mode & 2){
				for(uint8_t i=0; i<MAX_WS2811_LEDS; i++){
					// introduce correct pixel to address mapping
					led_values[i].g = dmx_buffer_in[local_dmx_addr++];
					led_values[i].r = dmx_buffer_in[local_dmx_addr++];
					led_values[i].b = dmx_buffer_in[local_dmx_addr++];
				}
			}
			// timing critical so disable interupts
			cli();
			ws2812_setleds(led_values,MAX_WS2811_LEDS);
			sei();

			dmx_change = 0;
			noDMX_timer = DMX_LOST_TIMEOUT;
		}else{
			if(!noDMX_timer){
				noDMX_timer = DMX_LOST_TIMEOUT;
				testSwitchMode();
			}
		}
	}		
}

