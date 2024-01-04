// NOTE: CONNECT WS2812's to PC6

#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>

#define WS2812BSIMPLE_IMPLEMENTATION
#define WS2812BSIMPLE_NO_IRQ_TWEAKING
#define NR_LEDS 29

#include "ws2812b_simple.h"

#include "color_utilities.h"


uint16_t phases[NR_LEDS];
int frameno;
volatile int tween = -NR_LEDS;

// Callbacks that you must implement.
uint32_t WS2812BLEDCallback( int ledno )
{
	uint8_t index = (phases[ledno])>>8;
	uint8_t rsbase = sintable[index];
	uint8_t rs = rsbase>>3;
	uint32_t fire = ((huetable[(rs+190)&0xff]>>1)<<16) | (huetable[(rs+30)&0xff]) | ((huetable[(rs+0)]>>1)<<8);
	uint32_t ice  = 0x7f0000 | ((rsbase>>1)<<8) | ((rsbase>>1));

	// Because this chip doesn't natively support multiplies, we are going to avoid tweening of 1..254.
	return TweenHexColors( fire, ice, ((tween + ledno)>0)?255:0 ); // Where "tween" is a value from 0 ... 255
}

int main()
{
	int k;
	SystemInit();

	// Enable GPIOD (for debugging)
	//RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	//GPIOD->CFGLR &= ~(0xf<<(4*0));
	//GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0);

	//GPIOD->BSHR = 1;	 // Turn on GPIOD0

	frameno = 0;

	for( k = 0; k < NR_LEDS; k++ ) phases[k] = k<<8;


	int tweendir = 0;

    uint8_t * data;

	while(1)
	{
		frameno++;

		if( frameno == 1024 )
		{
			tweendir = 1;
		}
		if( frameno == 2048 )
		{
			tweendir = -1;
			frameno = 0;
		}

		if( tweendir )
		{
			int t = tween + tweendir;
			if( t > 255 ) t = 255;
			if( t < -NR_LEDS ) t = -NR_LEDS;
			tween = t;
		}

		for( k = 0; k < NR_LEDS; k++ )
		{
			phases[k] += ((((rands[k&0xff])+0xf)<<2) + (((rands[k&0xff])+0xf)<<1))>>1;
		}

        data = (uint8_t *) WS2812BLEDCallback(k);

		WS2812BSimpleSend( GPIOD, 6, data, 24 );
	}
}

