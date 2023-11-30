/*
 * RP2040 PWM Tone
 * Tone generation library for Raspberry Pi Pico. Plays melodies
 * and chirping sounds via PWM through a buzzer or speaker.
 * By Turi Scandurra – https://turiscandurra.com/circuits
 * v1.0.0 - 2023.09.25
*/

#include "pwm-tone.h"   // Include the library
#include "melodies.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"

static alarm_id_t tone_a;
static alarm_id_t melody_a;
static alarm_id_t rest_a;

uint32_t clock;
uint16_t melody_repeat;
uint16_t melody_index;
uint16_t rest_duration = 10;
uint16_t tempo = 120;


int abs(int x){
    if(x < 0)
        return -x;
    return x;
}

void tone_init(struct tonegenerator_t* gen, uint8_t gpio){
    gen->gpio = gpio;
    gen->slice = pwm_gpio_to_slice_num(gpio);
    gen->channel = pwm_gpio_to_channel(gpio);
    gpio_init(gpio);
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    pwm_set_chan_level(gen->slice, gen->channel, 2048);
    clock = SYS_CLK_KHZ * 1000;
}

void tone(struct tonegenerator_t* gen, int freq, uint16_t duration) {
    if(freq != REST){
        _tone_pwm_on(gen, freq);
        if (tone_a) cancel_alarm(tone_a);
        tone_a = alarm_pool_add_alarm_in_ms(gen->alarm_pool, duration, _tone_complete, gen, true);
    }
}

void melody(struct tonegenerator_t* gen, struct note_t *notes, int8_t repeat){
    melody_repeat = repeat;
    melody_index = 0;
    struct melody_t mel;
    mel.notes = notes;
    gen->mel = mel;
    _melody_step(gen);
}

void set_tempo(uint16_t bpm){
    tempo = bpm;
}

void set_rest_duration(uint16_t duration){
    rest_duration = duration;
}

void stop_tone(struct tonegenerator_t* gen){
    pwm_set_enabled(gen->slice, false);
}

void stop_melody(struct tonegenerator_t* gen){
    if (tone_a) cancel_alarm(tone_a);
    if (melody_a) cancel_alarm(melody_a);
    pwm_set_enabled(gen->slice, false);
}

void _pwm_set_freq(struct tonegenerator_t* gen, float freq) {
    float divider = (float) clock / (freq * 10000.0);
    pwm_set_clkdiv(gen->slice, divider);
    pwm_set_wrap(gen->slice, 10000);
    pwm_set_gpio_level(gen->gpio, 5000);
}

void _tone_pwm_on(struct tonegenerator_t* gen, int freq){
    if(freq < NOTE_G1) {freq = NOTE_G1;}
    else if(freq > NOTE_FS9) {freq = NOTE_FS9;}
    pwm_set_enabled(gen->slice, false);
    _pwm_set_freq(gen, freq);
    pwm_set_enabled(gen->slice, true);
}

void _melody_step(struct tonegenerator_t* gen){
    struct melody_t mel = gen->mel;
    struct note_t note = mel.notes[melody_index];

    if (note.freq == MELODY_END){
        melody_index = 0;
        if(melody_repeat > 0){
            melody_repeat--;
        }
        if(melody_repeat != 0){
            _melody_step(gen);
        }
        
    } else {
        int8_t measure = note.measure;
        int whole_note = (60000 * 4) / tempo;
        int duration = whole_note / abs(measure);
        if (measure < 0) { // Dotted note
            duration *= 1.5;
        }
        _melody_tone(gen, note.freq, duration);
        melody_index++;
    }
}

void _melody_tone(struct tonegenerator_t *gen, int freq, uint16_t duration)
{
    if(freq != REST){ _tone_pwm_on(gen, freq); }
    if (melody_a) cancel_alarm(melody_a);
    melody_a = alarm_pool_add_alarm_in_ms(gen->alarm_pool, duration, _melody_note_complete, gen, true);
}

static int64_t _tone_complete(alarm_id_t id, void *user_data) {
    struct tonegenerator_t* gen = (struct tonegenerator_t*) user_data;
    pwm_set_enabled(gen->slice, false);
    return 0;
}

static int64_t _melody_note_complete(alarm_id_t id, void *user_data)
{
    struct tonegenerator_t* gen = (struct tonegenerator_t*) user_data;
    pwm_set_enabled(gen->slice, false);

    if(rest_duration > 0){
        if (rest_a) cancel_alarm(rest_a);
        rest_a = alarm_pool_add_alarm_in_ms(gen->alarm_pool, rest_duration, _rest_complete, user_data, true);
    } else {
        _melody_step(user_data);
    }
    return 0;
}

static int64_t _rest_complete(alarm_id_t id, void *user_data) {
    _melody_step(user_data);
    return 0;
}