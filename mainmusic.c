// original gameboy
#include <gb/gb.h>

#include "music.h"


UINT8 counter;

void timer_isr(){
	if(counter%5 == 0){
		tick_music();
	}
	counter++;
	counter %= 20;
}

void main() {
	counter = 0;
	NR52_REG = 0x80; // enable sound
	NR50_REG = 0x77; // full volume
	NR51_REG = 0xFF; // all channels

	init_music();
	// configure interrupt
	TIMA_REG = TMA_REG = 0x1A;
	TAC_REG = 0x4 | 0x0;//4096 Hz
	// enable timer interrupt
	disable_interrupts();
	add_TIM(timer_isr);
	enable_interrupts();
	set_interrupts(TIM_IFLAG);
}
