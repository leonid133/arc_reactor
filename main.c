/**
 * Copyright (c) 2019, Leonid Blokhin <lenin133@yandex.com>
 * ATtiny13/001
 * Iron Man Generator
 */

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define	LED_WITH	  PB3
#define	LED_BLUE	  PB1
#define	LED_CENTER	PB2
#define	LED_RED	    PB4

#define LFSR_SEED 	(91)
#define	DELAY		    (1)
#define DELAY_LONG  (300) 

#define	UPDATE	(1 << 0)
#define	MODE_1	(1 << 1)
#define	MODE_2	(1 << 2)
#define	MODE_3	(1 << 3)
#define	MODE_4	(1 << 4)
#define	MODE_5	(1 << 5)
#define	MODE_6	(1 << 6)
#define	MODE_7	(1 << 7)

#define PWM_DUTY_MIN		(1)
#define PWM_DUTY_MAX		(254)
#define PWM_DUTY_STEP		(2)
#define	PWM_DUTY_STEP_SMALL	(1)
#define KEY_UP			(1 << 1)
#define KEY_DOWN		(1 << 2)
static volatile uint8_t keys;

static volatile uint8_t timer_counter;
static volatile uint8_t timer_events;
static volatile uint8_t timer_seconds;
static volatile uint8_t timer_minutes;

static volatile uint8_t duty;

static void timer_init(void);
static void timer_handler(void);
static void timer_process(void);

static uint16_t
prng_lfsr16(void)
{
        static uint16_t cnt16 = LFSR_SEED;
        return (cnt16 = (cnt16 >> 1) ^ (-(cnt16 & 1) & 0xB400));
}

ISR(TIM0_COMPA_vect)
{
  timer_handler();
}


int
main(void)
{
  /* setup */
  	timer_init();

  	/* loop */
  	while (1) {
  		timer_process();
     // _delay_ms(DELAY);
  	}

}

void
timer_init(void)
{
  /* setup */
  DDRB = (1<<LED_RED) | (1<<LED_CENTER) | (1<<LED_BLUE) | (1<<LED_WITH); // set LED pins as OUTPUT
  PORTB = 0b00000000; // set all pins to LOW
  //TCCR0B |= _BV(CS02)|_BV(CS00);
  //TIMSK0 |= _BV(TOIE0);
  TCCR0A |= _BV(WGM01); // set timer counter mode to CTC
        TCCR0B |= _BV(CS01)|_BV(CS00); // set prescaler to 64 (CLK=1200000Hz/64/250=75Hz)
        OCR0A = 249; // set Timer's counter max value (250 - 1)
        TIMSK0 |= _BV(OCIE0A); // enable Timer CTC interrupt
	timer_counter = timer_seconds = timer_minutes = duty = 0; // reset counters
	timer_events = MODE_7; // first events
        sei(); // enable global interrupts
}

void
timer_handler(void)
{
	timer_counter++;
  if (timer_counter == 75) {
		timer_counter = 0;
		if (++timer_seconds == 60) {
			timer_seconds = 0;
			if (++timer_minutes == 30) {
				timer_minutes = 0;
			}
		}
    if (timer_minutes == 1) {
      timer_events |= MODE_1;
      timer_events &= ~MODE_2;
      timer_events &= ~MODE_3;
      timer_events &= ~MODE_4;
      timer_events &= ~MODE_5;
    } else if (timer_minutes == 5) {
      timer_events &= ~MODE_1;
      timer_events |= MODE_2;
      timer_events &= ~MODE_3;
      timer_events &= ~MODE_4;
      timer_events &= ~MODE_5;
    } else if (timer_minutes == 10) {
      timer_events &= ~MODE_1;
      timer_events &= ~MODE_2;
      timer_events |= MODE_3;
      timer_events &= ~MODE_4;
      timer_events &= ~MODE_5;
    } else if (timer_minutes == 15) {
      timer_events &= ~MODE_1;
      timer_events &= ~MODE_2;
      timer_events &= ~MODE_3;
      timer_events |= MODE_4;
      timer_events &= ~MODE_5;
    } else if (timer_minutes == 20) {
      if (timer_seconds < 15) {
        timer_events = MODE_5;
      } else if (timer_seconds > 15 && timer_seconds < 50) {
        timer_events = MODE_6;
      } else {
        timer_events = MODE_7;
      }
    }
	}
  
  
 if (timer_events & MODE_4) {
    if (timer_counter % duty) {
  		PORTB |= _BV(LED_RED);
  	} else {
  		PORTB &= ~_BV(LED_RED);
  	}
    if (timer_counter % duty) {
      PORTB &= ~_BV(LED_CENTER);
      PORTB &= ~_BV(LED_WITH);
    } else {
      PORTB |= _BV(LED_CENTER);
      PORTB |= _BV(LED_WITH);
    }
  }
}

void
timer_process(void)
{
    if (timer_events & MODE_1) {
      if ((keys & KEY_UP) && duty < (PWM_DUTY_MAX-1) && (prng_lfsr16() & 1)) {
    		duty += PWM_DUTY_STEP;
    	} else {
        keys = KEY_DOWN;
      }

    	if ((keys & KEY_DOWN) && duty > PWM_DUTY_MIN && (prng_lfsr16() & 1)) {
    		duty -= PWM_DUTY_STEP;
    	} else {
        keys = KEY_UP;
      }
      
        PORTB |= _BV(LED_WITH);
        PORTB &= ~_BV(LED_CENTER);
        PORTB &= ~_BV(LED_BLUE);
        _delay_loop_2(PWM_DUTY_MAX);
        
        PORTB &= ~_BV(LED_WITH);
        PORTB |= _BV(LED_CENTER);
        PORTB |= _BV(LED_BLUE);
        _delay_loop_2(PWM_DUTY_MAX-duty);
        
    } else if (timer_events & MODE_2) {
      PORTB |= _BV(LED_BLUE);
      if (prng_lfsr16() & 1) {
              PORTB |= _BV(LED_CENTER);
              PORTB |= _BV(LED_WITH);
      } else {
              PORTB &= ~_BV(LED_CENTER);
              PORTB &= ~_BV(LED_WITH);
		  }
      _delay_ms(DELAY);
		} else if (timer_events & MODE_3) {
      PORTB &= ~_BV(LED_BLUE);
      PORTB &= ~_BV(LED_WITH);
      if ((keys & KEY_UP) && duty < (PWM_DUTY_MAX-1)) {
    		duty += PWM_DUTY_STEP_SMALL;
    	} else {
        keys = KEY_DOWN;
      }

    	if ((keys & KEY_DOWN) && duty > PWM_DUTY_MIN) {
    		duty -= PWM_DUTY_STEP_SMALL;
    	} else {
        keys = KEY_UP;
      }
      
        PORTB &= ~_BV(LED_RED);
        PORTB |= _BV(LED_CENTER);
        _delay_loop_2(PWM_DUTY_MAX);
        
        PORTB |= _BV(LED_RED);
        PORTB &= ~_BV(LED_CENTER);
        _delay_loop_2(PWM_DUTY_MAX-duty);
        
      
		} else if (timer_events & MODE_4) {
      //PORTB &= ~_BV(LED_WITH);
      PORTB &= ~_BV(LED_BLUE);
      if ((keys & KEY_UP) && duty < PWM_DUTY_MAX) {
    		duty += PWM_DUTY_STEP;
    	} else {
        keys = KEY_DOWN;
      }

    	if ((keys & KEY_DOWN) && duty > PWM_DUTY_MIN) {
    		duty -= PWM_DUTY_STEP;
    	} else {
        keys = KEY_UP;
      }
    } else if (timer_events & MODE_5) {
      PORTB &= ~_BV(LED_RED);
      if (prng_lfsr16() & 1) {
              PORTB |= _BV(LED_WITH);
              PORTB &= ~_BV(LED_BLUE);
      } else {
              PORTB &= ~(_BV(LED_WITH));
              PORTB |= _BV(LED_BLUE);
		  }
      PORTB |= _BV(LED_CENTER);
      _delay_ms(DELAY);
      PORTB &= ~_BV(LED_BLUE);
      PORTB &= ~_BV(LED_WITH);
      PORTB &= ~_BV(LED_CENTER);
      _delay_ms(DELAY_LONG);
    } else if (timer_events & MODE_6) {
      PORTB &= ~_BV(LED_WITH);
      PORTB ^= _BV(LED_CENTER);
      _delay_ms(DELAY_LONG);
    } else if (timer_events & MODE_7) {
      PORTB |= _BV(LED_WITH);
      PORTB |= _BV(LED_BLUE);
      PORTB |= _BV(LED_CENTER);
    }

}
