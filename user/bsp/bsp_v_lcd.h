#ifndef __BSP_V_LCD_H
#define __BSP_V_LCD_H

#include "t_debug.h"

#define  V_LCD_W		480
#define	 V_LCD_H		272
void V_lcd_init(void);
void drawpoint(int x,int y,uint16_t c);
uint16_t readpoint(int x,int y);
#endif
