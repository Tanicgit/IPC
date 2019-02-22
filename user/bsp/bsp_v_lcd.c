#include "bsp_v_lcd.h"
#include "bsp_base.h"

#include "myheap4.h"
static uint16_t *LCD_RAM;
void V_lcd_init()
{
	LCD_RAM = staticMalloc(V_LCD_W*V_LCD_H*2);
	if(LCD_RAM==NULL)
	{
		__aeabi_assert("LCD_RAM Malloc Err",__FILE__,__LINE__);
	}
}

void drawpoint(int x,int y,uint16_t c)
{
	*(LCD_RAM+y*V_LCD_W+x) = c;
}

uint16_t readpoint(int x,int y)
{
  return *(LCD_RAM+y*V_LCD_W+x);
}










