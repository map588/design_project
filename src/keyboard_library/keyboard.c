#include "keyboard.h"
#include "definitions.h"
#include "pico/printf.h"
#include "keyboard.pio.h"

#define DATA_PIN 14
#define CLOCK_PIN 15


char scan_codes[] = {
	'/','*','-','7','8','9','+','4','5','6','1','2','3','\n','0','.',
	'%','&','!','A','^','B','|','<','C','>','D','v','E','\n','F','\b'
};


static PIO pio = pio0;
static uint sm = 0;

uint32_t key_packet;

void get_code()
{
	uint32_t input = pio_sm_get_blocking(pio, sm);

	uint8_t key = input & 0x1F;


	bool numlock   = 0;
	bool pressed   = 0;
	
	char output = scan_codes[key];
	
	uint32_t key_packet = assemble_packet(KEYPRESS, 0, (uint8_t)output, 0);
	multicore_fifo_push_blocking(key_packet);
	}


void keyboard_init()
{
	float freq_khz = 32.0f;
	pio_gpio_init(pio, DATA_PIN);
	pio_gpio_init(pio, CLOCK_PIN);
	gpio_pull_up(DATA_PIN);
	gpio_pull_up(CLOCK_PIN);

	sm = pio_claim_unused_sm(pio, true);

	// Set up the state machine we're going to use to receive them.
    uint offset = pio_add_program(pio, &keyboard_pio_program);

    pio_sm_config c = keyboard_pio_program_get_default_config(offset);
    sm_config_set_in_pins(&c, DATA_PIN);
    sm_config_set_in_shift(&c, true, true, 8);

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
