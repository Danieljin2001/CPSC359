#include <stdio.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <bcm_host.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define GPIO_OFFSET 0x200000
#define TIMER_OFFSET 0x00003000

//initialize variables for timer
volatile unsigned int *timer;
void *timerMap;
int fdTimer;

//initialize variables for gpio
volatile unsigned int *gpio;
void *gpioMap;
int fdGPIO;

#define TX 9  		//MISO - output speakers, pin #9
#define RX 10 		//MOSI - input speakers, pin #10

#define GPFSEL0 0 	//regiser for when using pin #9 (MISO)
#define GPFSEL1 1 	//register for when using pin #10 (MOSI)

#define GPSET0 7	//set 0 register
#define GPCLR0 10	//clear 0 register

#define GPLEV0 13	//register for reading value of pin #10

#define GPPUD 37	//pull-up/pull-down register
#define GPPUDCLK0 38	//pull-up/pull-down clock register

#define CS      0	//system timer control/status register
#define CLO     1	//system timer counter lower 32 bits register
#define CHI     2	//system timer counter higher 32 bits register
#define C0      3	//system timer compare 0 register
#define C1      4	//system timer compare 1 register
#define C2      5	//system timer compare 2 register
#define C3      6	//system timer compare 3 register

//this is used to initialize the US100 MOSI and MOSI gpios
void initUS100() {
	unsigned peripheralBase = bcm_host_get_peripheral_address();
	fdGPIO = open("/dev/mem", O_RDWR|O_SYNC);
	gpioMap = (unsigned int *)mmap(
		NULL,
		BLOCK_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		fdGPIO,
		peripheralBase + GPIO_OFFSET
	);
	
	if ( gpioMap == MAP_FAILED ) {
		perror( "mmap" );
		return;
	}
	
	gpio = (volatile unsigned int *) gpioMap;
	
	register unsigned int r;
	
	//Setting the functions for registers
	
	//r = GPFSEL0, for gpio 9 (pin 9, TX)
	r = gpio[GPFSEL0];
	
	//clear bits
	//3 bits * (9 mod 10) = 27 
	//replace bits with 0's while keeping every other value in the register the same
	r &= ~(0x7 << 27); 
	
	//set pin 9 to output (001)
	r |= (0x1 << 27);
	
	//write the modified bit pattern back into register
	gpio[GPFSEL0] = r;
	
	//r = GPFSEL1, for gpio 10(pin 10, RX)
	r = gpio[GPFSEL1];
	
	//clear bits
	//3 bits * (10 mod 10) = 0 
	//replace bits with 0's while keeping every other value in the register the same
	//this also is used to set pin #10 to input (000)
	r &= ~(0x7); 
	
	//write the modified bit pattern back into register
	gpio[GPFSEL1] = r;
	
	// Disable the pull-up/pull-down control line for GPIO pins. We follow the
	// procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
	// internal pull-up and pull-down resistor isn't needed for an output pin.

	// Disable pull-up/pull-down by setting bits 0:1
	// to 00 in the GPIO Pull-Up/Down Register 
	gpio[GPPUD] = 0x0;

	// Wait 150 cycles to provide the required set-up time 
	// for the control signal
	r = 150;
	while (r--) {
	  asm volatile("nop");
	}

	// for gpio 9 use << 9
	gpio[GPPUDCLK0] = (0x1 << TX);

	// Wait 150 cycles to provide the required hold time
	// for the control signal
	r = 150;
	while (r--) {
	  asm volatile("nop");
	}

	// Clear all bits in the GPIO Pull-Up/Down Clock Register
	// in order to remove the clock
	gpio[GPPUDCLK0] = 0;
	
	//clearing pin #9 just in case
	gpio[GPCLR0] = 1 << TX;
}

//unmap the memory and close the memory device
void freeUS100 () {
	munmap( gpioMap, BLOCK_SIZE );
	close( fdGPIO );
}	

//set pin #9
void TXon() {
	gpio[GPSET0] = 1 << TX;
}

//clear pin #9
void TXoff() {
	gpio[GPCLR0] = 1 << TX;
}

//read actual value of pin #10 (1 or 0)
int readRX() {
	register unsigned int r = gpio[GPLEV0];
	//right shift 10 times to get the value of pin #10
	//then AND it with 0x1 to only get the value of the LSB of r
	r = ((r >> RX) & 0x1);
	return r;
}

//this method is to initialize the BCM system timer register
//this code is gotten from Dr. Boyd's timer example in D2L
void initTimer() {
    unsigned peripheralBase = bcm_host_get_peripheral_address();
    fdTimer = open("/dev/mem", O_RDWR|O_SYNC);
    timerMap = (unsigned int *)mmap(
        NULL,
        BLOCK_SIZE,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        fdTimer,
        peripheralBase + TIMER_OFFSET
    );
    
    if ( timerMap == MAP_FAILED ) {
        perror( "mmap" );
        return;
    }
    
    timer = (volatile unsigned int *) timerMap;
}

void freeTimer() {
    munmap( timerMap, BLOCK_SIZE );
    close( fdTimer );
}

unsigned long long getSystemTimerCounter() {
 // from  https://embedded-xinu.readthedocs.io/en/latest/arm/rpi/BCM2835-System-Timer.html

    unsigned int h=-1, l;
    
    // we must read MMIO area as two separate 32 bit reads
    h = timer[CHI];
    l = timer[CLO];

    // we have to repeat it if high word changed during read
    //   - low low counter rolled over
    if ( h != timer[CHI] ) {
        h = timer[CHI];
        l = timer[CLO];
     }
    // compose long long int value
    return ((unsigned long long) h << 32) | (unsigned long long)l;
}


