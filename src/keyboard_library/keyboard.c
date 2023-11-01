#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "keyboard.h"
#include "keyboard.pio.h"

#define DATA_PIN 14
#define CLOCK_PIN 15

PIO pio = pio1;
uint sm = 0;


void get_code()
{
    int got = pio_sm_get_blocking(pio, sm);
    int data = (got >> 22) & 0xff; //right justify

  static int shifted = 0;
	static int released = 0;
	if (released)  // last key was a break code
	{
		switch (data)
		{
			case 0x59:
			case 0x12:
				shifted = 0;
				break;
			default:
				break;
		}
		released = 0;
	}
	else
		switch (data)
		{
			case 0x59:  // right shift
            case 0x12:  // left shift
                shifted = 1;
				break;
			case 0xF0:  // break code
				released = 1;
				break;
			default:
                int i;
				for (i = 0 ; data != scan_codes[i][0] ; i++)
				    if (scan_codes[i][0] == 0)
				    break;

			        if (data == scan_codes[i][0])
				         if (shifted)
				        	input = (scan_codes[i][2]);
				         else
					        input = (scan_codes[i][1]);

                    packet = assemble_packet(KEYPRESS, 0, 0, (uint8_t) input, 0);
                    multicore_fifo_push_blocking(packet);
                    break;
		}
}

void keyboard_init()
{
    // Set up the state machine we're going to use to receive them.
    uint offset = pio_add_program(pio, &keyboard_pio_program);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, DATA_PIN);
    sm_config_set_in_shift(&c, true, true, 11);

    pio_sm_set_consecutive_pindirs(pio, sm, DATA_PIN, 2, false);
    pio_set_irq0_source_enabled(pio, pis_sm0_rx_fifo_not_empty, true);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    irq_set_exclusive_handler(PIO0_IRQ_0, &get_code);
    irq_set_enabled(PIO0_IRQ_0, true);

    printf("\nstarting keypad\n");
}
