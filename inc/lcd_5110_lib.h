#ifndef LCD_5110_LIB_H
#define LCD_5110_LIB_H

/*#define I2C_BARO_SCL_PIN 13 //these are defined in bitbang header!
#define I2C_BARO_SDA_PIN 6*/
#/*define LCD_5110_RST_PIN 5
#define LCD_5110_CE_PIN 6
#define LCD_5110_DC_PIN 13
#define LCD_5110_DIN_PIN 19
#define LCD_5110_CLK_PIN 26
*/

#define LCD_5110_RST_OUTPUT GPIOSetDir(1, 28, 1)
#define LCD_5110_CE_OUTPUT GPIOSetDir(1, 29, 1)
#define LCD_5110_DC_OUTPUT GPIOSetDir(1, 31, 1)
#define LCD_5110_DIN_OUTPUT GPIOSetDir(1, 22, 1)
#define LCD_5110_CLK_OUTPUT GPIOSetDir(1, 20, 1)

#define LCD_5110_RST_HIGH GPIOSetValue(1, 28, 1)
#define LCD_5110_CE_HIGH GPIOSetValue(1, 29, 1)
#define LCD_5110_DC_HIGH GPIOSetValue(1, 31, 1)
#define LCD_5110_DIN_HIGH GPIOSetValue(1, 22, 1)
#define LCD_5110_CLK_HIGH GPIOSetValue(1, 20, 1)

#define LCD_5110_RST_LOW GPIOSetValue(1, 28, 0)
#define LCD_5110_CE_LOW GPIOSetValue(1, 29, 0)
#define LCD_5110_DC_LOW GPIOSetValue(1, 31, 0)
#define LCD_5110_DIN_LOW GPIOSetValue(1, 22, 0)
#define LCD_5110_CLK_LOW GPIOSetValue(1, 20, 0)

//#define LCD_5110_DELAY delay(1);
#define LCD_5110_DELAY

extern void lcd_5110_init();



#endif
