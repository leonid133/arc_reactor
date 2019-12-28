#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB0

int
main(void)
{
	/*setup*/
	DDRB = 0b00011111;
	PORTB = 0b00011111;
	
	while(1) {
		PORTB ^= 0b00011111;
		_delay_ms(500);
	}
}
