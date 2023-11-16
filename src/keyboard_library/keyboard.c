#include "keyboard.h"
#include "definitions.h"
#include "pico/printf.h"
#include "keyboard.pio.h"

#define DATA_PIN 14
#define CLOCK_PIN 15


char scan_codes[] = //with an offset of 0x69, these can be directy index for scan codes
{
	'1',
	' ',
	'4',
	'7',
	' ', //-
	' ', //-
	' ', //-
	'0',
	'.',
	'2',
	'5',
	'6',
	'8',
	'~', //numlock
	' ', //-
	'+',
	'3',
	'-',
	'*',
	'9',
};

char special_codes[] = //These follow 0xE0 with offset 0x69
{
	' ', // end
	' ', // -
	'<', // left arrow
	' ', // home
	' ', // -
	' ', // -
	' ', // -
	' ', // insert
	' ', // delete
	'v', // down arrow
	' ', // -
	'>', // right arrow
	'^', // up arrow
	' ', // -
	' ', // -
	' ', // -
	' ', // -
	' ', // page down
	' ', // -
	' ', // -
	' ', // page up
};
char outliers [] = //These follow 0xE0, shift 4 minus 4 gives index 0
{ 
	'/',
   '\n',
};

static PIO pio = pio0;
static uint sm = 0;

uint32_t key_packet;

void get_code()
{
	pio_interrupt_clear(pio, sm);

	static char bytes[4];

	uint32_t input = pio_sm_get_blocking(pio, sm);

	bytes[0] = (input & 0xFF000000) >> 24;
	bytes[1] = (input & 0x00FF0000) >> 16;
	bytes[2] = (input & 0x0000FF00) >> 8;
	bytes[3] = (input & 0x000000FF);

	bool numlock   = 0;
	bool pressed   = 0;
	
	char in;

	char output;
	for(int i = 1; i < 4; i++){
		uint8_t prev_data = bytes[i-1];
		uint8_t data = bytes[i];

		switch (prev_data)
			{
			case 0xF0:  // break code
				pressed  = 0;
				break;
			case 0x77:
				numlock  = !numlock;
				break;
			default:
				switch(prev_data)
				{
					case 0xE0:
						char in = (data >> 4) - 4;
						if(in < 2)
						output = outliers[in];
						break;
					default:
						in = prev_data - 0x69;

					if (numlock && in < 21)
						output = special_codes[prev_data - 0x69];
					else if(in  < 20)
						output = scan_codes[prev_data - 0x69];

					if(output == '<' || output == '>' || output == '^' || output == 'v'){
						key_packet = assemble_packet(KEYPRESS, 1, 0, (uint8_t)output, 0);
						multicore_fifo_push_blocking(key_packet);
					}
					else if(pressed == 0 && output != ' '){
						key_packet = assemble_packet(KEYPRESS, 0, 0, (uint8_t)output, 0);
						multicore_fifo_push_blocking(key_packet);
					}
					break;
				}
				pressed = 1;
				break;
			}
		}
	}


void keyboard_init()
{
	gpio_init(DATA_PIN);
	gpio_init(CLOCK_PIN);
	gpio_pull_up(DATA_PIN);
	gpio_pull_up(CLOCK_PIN);

	sm = pio_claim_unused_sm(pio, true);
	
	// Set up the state machine we're going to use to receive them.
    uint offset = pio_add_program(pio, &keyboard_pio_program);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, DATA_PIN);
    sm_config_set_in_shift(&c, true, true, 32);

    pio_sm_set_consecutive_pindirs(pio, sm, DATA_PIN, 2, false);
    pio_set_irq0_source_enabled(pio, pis_sm0_rx_fifo_not_empty, true);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    irq_set_exclusive_handler(PIO0_IRQ_0, &get_code);
    irq_set_enabled(PIO0_IRQ_0, true);

    printf("\nstarting keypad\n");
}
