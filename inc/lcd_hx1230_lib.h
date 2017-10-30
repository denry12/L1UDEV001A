#ifndef LCD_HX1230_LIB_H
#define LCD_HX1230_LIB_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool (*sendSPIpacket)(); // Pretty sure there should be a how to contents inside marked too!
	bool (*enableCS)(); //sets CS voltage low
	bool (*disableCS)(); //sets CS voltage high
	bool (*enableBL)(); //
	bool (*disableBL)(); //
	bool (*enableReset)(); //
	bool (*disableReset)(); //

} hx1230_instance;

#endif
