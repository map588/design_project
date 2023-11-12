
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "definitions.h"

// PIO program for scanning the keyboard matrix
void keyboard_init();

char scan_codes[][3] =
{
	0x0e,'|',' ',
	0x15,'q','Q',
	0x16,'1','!',
	0x1a,'z','Z',
	0x1b,'s','S',
	0x1c,'a','A',
	0x1d,'w','W',
	0x1e,'2','"',
	0x21,'c','C',
	0x22,'x','X',
	0x23,'d','D',
	0x24,'e','E',
	0x25,'4','$',
	0x26,'3','#',
	0x29,' ',' ',
	0x2a,'v','V',
	0x2b,'f','F',
	0x2c,'t','T',
	0x2d,'r','R',
	0x2e,'5','%',
	0x31,'n','N',
	0x32,'b','B',
	0x33,'h','H',
	0x34,'g','G',
	0x35,'y','Y',
	0x36,'6','&',
	0x39,',',';',
	0x3a,'m','M',
	0x3b,'j','J',
	0x3c,'u','U',
	0x3d,'7','/',
	0x3e,'8',' ',
	0x41,',',';',
	0x42,'k','K',
	0x43,'i','I',
	0x44,'o','O',
	0x45,'0','=',
	0x46,'9',' ',
	0x49,'.',':',
	0x4a,'-','_',
	0x4b,'l','L',
	0x4d,'p','P',
	0x4e,'+','*',
	0x55,'\\',' ',
	0x5a,'\r','\r',
	0x5d,'\'',' ',
	0x61,'<','>',
	0x66,'\b','\b',
	0x69,'1',' ',
	0x6b,'4',' ',
	0x6c,'7',' ',
	0x70,'0',' ',
	0x71,',',' ',
	0x72,'2',' ',
	0x73,'5',' ',
	0x74,'6',' ',
	0x75,'8',' ',
	0x79,'+',' ',
	0x7a,'3',' ',
	0x7b,'-',' ',
	0x7c,'*',' ',
	0x7d,'9',' ',
	0,'\0','\0',
};

#endif