#include "ch32fun.h"
#include "iSLER.h"
#include <stdio.h>

#ifdef CH570_CH572 // this comes from iSLER.h
#define LED PA9
#else
#define LED PA8
#endif
#define LL_TX_POWER_0_DBM 0x12
#define PHY_MODE          PHY_1M

#define REPORT_ALL 1
uint8_t adv[] = {0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // MAC (reversed)
				 0x03, 0x19, 0x00, 0x00, // 0x19: "Appearance", 0x00, 0x00: "Unknown"
				 0x06, 0x09, 'R', 'X', ':', '?', '?'}; // 0x09: "Complete Local Name"
uint8_t adv_channels[] = {37,38,39};
uint8_t hex_lut[] = "0123456789ABCDEF";

void blink(int n) {
	for(int i = n-1; i >= 0; i--) {
		funDigitalWrite( LED, FUN_LOW ); // Turn on LED
		Delay_Ms(33);
		funDigitalWrite( LED, FUN_HIGH ); // Turn off LED
		if(i) Delay_Ms(33);
	}
}

void incoming_frame_handler() {
	uint8_t *frame = (uint8_t*)LLE_BUF;
	printf("RSSI:%d len:%d MAC:", frame[0], frame[1]);
	for(int i = 7; i > 2; i--) {
		printf("%02x:", frame[i]);
	}
	printf("%02x data:", frame[2]);
	for(int i = 8; i < frame[1] +2; i++) {
		printf("%02x ", frame[i]);
	}
	printf("\n");

	// advertise reception of a FindMy frame
	if(REPORT_ALL || (frame[8] == 0x1e && frame[10] == 0x4c)) {
		adv[sizeof(adv) -2] = hex_lut[(frame[7] >> 4)];
		adv[sizeof(adv) -1] = hex_lut[(frame[7] & 0xf)];
		for(int c = 0; c < sizeof(adv_channels); c++) {
			Frame_TX(adv, sizeof(adv), adv_channels[c], PHY_MODE);
		}
	}
}

int main()
{
	SystemInit();

	funGpioInitAll();
	funPinMode( LED, GPIO_CFGLR_OUT_2Mhz_PP );

	RFCoreInit(LL_TX_POWER_0_DBM);
	uint8_t frame_info[] = {0xff, 0x10}; // PDU, len

	blink(5);
	printf(".~ ch32fun iSLER ~.\n");

	for(int c = 0; c < sizeof(adv_channels); c++) {
		Frame_TX(adv, sizeof(adv), adv_channels[c], PHY_MODE);
	}

	while(1) {
		Frame_RX(frame_info, 37, PHY_MODE);
		while(!rx_ready);

		blink(1);
		incoming_frame_handler();
	}
}
