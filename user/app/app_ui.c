#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "t_debug.h"
#include "bsp.h" 
#include "GUI.h"

#include "t_debug.h"
#include "t_shell.h"
#include "main.h"

#include "lwip/dhcp.h"

#include "myheap4.h"



GUI_BITMAP bm0 ;
//typedef struct {
//  U16P XSize;
//  U16P YSize;
//  U16P BytesPerLine;
//  U16P BitsPerPixel;
//  const U8 * pData;
//  const GUI_LOGPALETTE * pPal;
//  const GUI_BITMAP_METHODS * pMethods;
//} GUI_BITMAP;
void * creatbmp(int xsize,int ysize,int bpp)
{
	void *p;
	p = staticMalloc(xsize*ysize*bpp);
	if(p==NULL)return NULL ;

	bm0.XSize = xsize;
	bm0.YSize = ysize;
	bm0.BytesPerLine = xsize*bpp;
	bm0.BitsPerPixel = bpp;
	bm0.pData = p;
	bm0.pPal = NULL;
	bm0.pMethods = GUI_DRAW_BMPM565;
	return p;
}




#include "bsp_ov7670.h"
#include "fsl_ov7670.h"
extern GUI_CONST_STORAGE GUI_BITMAP bm1;

extern uint32_t activeFrameAddr;
extern uint32_t inactiveFrameAddr;
extern camera_receiver_handle_t cameraReceiver;

void display(uint8_t *p)
{
//		uint32_t i=0;
//		Ac_log("buf=[");
//		for(i=0;i<10;i++)
//		Ac_log("%02x ",*p++);
//		Ac_log("]\r\n");	
		memcpy((U8*)bm0.pData,(uint32_t*)p,320*240*2);
//		GUI_DrawBitmap(&bm0,0,0);	
}
__IO	uint32_t FPS_cnt=0;
void UI_app()
{
	
	GUI_SetBkColor(GUI_BLUE);
	GUI_SetColor(GUI_RED);
	GUI_Clear();
	
	creatbmp(320,240,2);
//	memcpy((U8*)bm0.pData,bm1.pData,480*272*2);
//	GUI_DrawBitmap(&bm0,0,0);
	
	bsp_InitCamera();
	
	display((void*)activeFrameAddr);
  for(;;)
  {
			
		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);	
		activeFrameAddr = inactiveFrameAddr;
		/* Wait to get the full frame buffer to show. */
		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &inactiveFrameAddr))
		{
		}	
		display((void*)inactiveFrameAddr);
		FPS_cnt++;
	//	vTaskDelay(20);
  }
}
