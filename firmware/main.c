/********************************
* APA102-POV                 	*
* MIT License               	*
* Copyright 2015 - Mike Szczys  *
* http://jumptuck.com 	    	*
*				                *
********************************/

#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>


/************** Setup bitbang LEDs ********************/
#define LED_DDR     DDRB
#define LED_PORT    PORTB     
#define LED_DATA    (1<<PB3)
#define LED_CLOCK   (1<<PB5)

/************** Setup input buttons *********************/
#define BUT_DDR     DDRD
#define BUT_PORT    PORTD
#define BUT_PIN     PIND
#define BUT_SW1    (1<<PD5)
#define BUT_SW2     (1<<PD6)
#define BUT_SW3     (1<<PD7)

/**************** Prototypes *************************************/
void POST(void);
void startFrame(void);
void endFrame(void);
void pushPixel(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b);
void shiftByte(uint8_t data);
void shiftZero(void);
void shiftOne(void);
void init_IO(void);
void init_interrupts(void);
/**************** End Prototypes *********************************/

void POST(void) {
    //Bit Bang the LEDs to ensure they work despite hardware SPI
    
    startFrame();
    
    //Shift Red
    pushPixel(0xFF, 0xFF, 0x00, 0x00);
    //Shift Green
    pushPixel(0xFF, 0x00, 0xFF, 0x00);
    //Shift Blue
    pushPixel(0xFF, 0x00, 0x00, 0xFF);

    endFrame();
    
}

void startFrame(void) {
    //Start frame (0*32);
    shiftByte(0x00);    shiftByte(0x00);   shiftByte(0x00);   shiftByte(0x00);
}

void endFrame(void) {
    //End frame (1*32);
    shiftByte(0xFF);    shiftByte(0xFF);   shiftByte(0xFF);   shiftByte(0xFF);
}

void pushPixel(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b) {
    //Brightness only uses 5 LSB. 3 MSB must all be 1
    shiftByte(0b11100000 | brightness);    shiftByte(b);   shiftByte(g);   shiftByte(r);
}
void shiftByte(uint8_t data) {
    for (uint8_t i=0; i<8; i++) {
        if (data & (1<<(7-i))) { shiftOne(); }
        else { shiftZero(); }
    }
}

void shiftZero(void) {
    LED_PORT &= ~LED_DATA;    //low
    LED_PORT |= LED_CLOCK;                  //Raise Clock to shift in Data
    LED_PORT &= ~LED_CLOCK;                  //Clock low
}

void shiftOne(void) {
    LED_PORT |= LED_DATA;                   //Data high
    LED_PORT |= LED_CLOCK;                  //Raise Clock to shift in Data
    LED_PORT &= ~LED_CLOCK;                  //Clock low
}

void init_IO(void) {
    //BitBang LEDS
    LED_DDR |= LED_DATA | LED_CLOCK;
    LED_PORT &= ~(LED_DATA | LED_CLOCK);       //start low
    
    //Buttons
    BUT_DDR &= ~(BUT_SW1 | BUT_SW2 | BUT_SW3);      //Set as input
    BUT_PORT |= BUT_SW1 | BUT_SW2 | BUT_SW3;      //Enable pull-up
}

void init_interrupts(void) {

}

void fadeForever(void) {
    uint8_t fade = 0;
    while(1)
    {
        if (++fade > 31) { fade = 0; }
        startFrame();
        pushPixel(fade, 0xFF, 0x00, 0x00);
        pushPixel(fade, 0x00, 0xFF, 0x00);
        pushPixel(fade, 0x00, 0x00, 0xFF);
        endFrame();
        _delay_ms(20);
    }
}

int main(void)
{
    init_IO();
    init_interrupts();

    _delay_ms(200);

    POST();
    
    while (BUT_PIN & BUT_SW3) {
        ; //Loop until the button is pressed
    }

    uint8_t cycleRed = 0xFF;
    uint8_t cycleGreen = 0;
    uint8_t cycleBlue = 0;
    /*
    128, 0, 0
    120, 8, 0
    112, 16, 0
    104, 24, 0
    96, 32, 0
    88, 40, 0
    80, 48, 0
    72, 56, 0
    64, 64, 0
    */

    while(1)
    {
        //fadeForever();
        //down red
        
        while (cycleRed > 0) {
            cycleRed -= 1;
            cycleGreen = 0xFF - cycleRed;
        
            startFrame();
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            endFrame();

            _delay_ms(2);
        }
        
        while (cycleGreen > 0) {
            cycleGreen -= 1;
            cycleBlue = 0xFF - cycleGreen;
        
            startFrame();
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            endFrame();

            _delay_ms(2);
        }
        
        while (cycleBlue > 0) {
            cycleBlue -= 1;
            cycleRed = 0xFF - cycleBlue;
        
            startFrame();
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            pushPixel(5, cycleRed, cycleGreen, cycleBlue);
            endFrame();

            _delay_ms(2);
        }
        //down green
        //down blue
        
    }
}
