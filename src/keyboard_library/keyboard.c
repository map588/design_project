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
	'e', // end
	' ', // -
	'<', // left arrow
	'h', // home
	' ', // -
	' ', // -
	' ', // -
	'i', // insert
	'd', // delete
	'v', // down arrow
	' ', // -
	'>', // right arrow
	'^', // up arrow
	' ', // -
	' ', // -
	' ', // -
	' ', // -
	'p', // page down
	' ', // -
	' ', // -
	'u', // page up
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
	uint32_t input = pio_sm_get_blocking(pio, sm);

	uint8_t bytes[2];
	bytes[0] = (input >> 22) & 0xFF;
	bytes[1] = (input >> 12) & 0xFF;


	bool numlock   = 0;
	bool pressed   = 0;
	
	uint8_t in;

	char output;
	// for(int i = 1; i < 4; i++){
	// 	uint8_t data = bytes[i-1];
	// 	uint8_t data_next = bytes[i];


		switch (bytes[0])
			{
			case 0xF0:  // break code
				pressed  = 0;
				break;
			case 0x77:
				numlock  = !numlock;
				break;
			default:
				switch(bytes[0])
				{
					case 0xE0:
						in = (bytes[1] >> 4) - 4;
						if(in < 2)
						output = outliers[in];
						break;
					default:
					    in = bytes[0] - 0x69;

					if (numlock && in < 21)
						output = special_codes[bytes[0] - 0x69];
					else if(in  < 20)
						output = scan_codes[bytes[0] - 0x69];

					if(output == '<' || output == '>' || output == '^' || output == 'v'){
						key_packet = assemble_packet(KEYPRESS, 0, (uint8_t)output, 1);
						multicore_fifo_push_blocking(key_packet);
					}
					else if(pressed == 0 && output != ' '){
						key_packet = assemble_packet(KEYPRESS, 0, (uint8_t)output, 0);
						multicore_fifo_push_blocking(key_packet);
					}
					break;
				}
				pressed = 1;
				break;
			}
	//	}
	}


void keyboard_init()
{
	float freq_khz = 25.8f;
	pio_gpio_init(pio,DATA_PIN);
	pio_gpio_init(pio, CLOCK_PIN);
	gpio_pull_up(DATA_PIN);
	gpio_pull_up(CLOCK_PIN);

	sm = pio_claim_unused_sm(pio, true);

	// Set up the state machine we're going to use to receive them.
    uint offset = pio_add_program(pio, &keyboard_pio_program);

    pio_sm_config c = keyboard_pio_program_get_default_config(offset);
    sm_config_set_in_pins(&c, DATA_PIN);
    sm_config_set_in_shift(&c, true, true, 22);

    pio_sm_set_consecutive_pindirs(pio, sm, DATA_PIN, 2, false);
    pio_set_irq0_source_enabled(pio, pis_sm0_rx_fifo_not_empty, true);

	float div = (float)SYS_CLK_KHZ / freq_khz;
	sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    irq_set_exclusive_handler(PIO0_IRQ_0, &get_code);
    irq_set_enabled(PIO0_IRQ_0, true);

    printf("\nstarting keypad\n");
}
