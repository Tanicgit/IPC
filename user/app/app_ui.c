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

#define  X_SIZE		320
#define	 Y_SIZE		240


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
//void * creatbmp(int xsize,int ysize,int bpp)
//{
//	void *p;
//	p = staticMalloc(xsize*ysize*bpp);
//	if(p==NULL)return NULL ;

//	bm0.XSize = xsize;
//	bm0.YSize = ysize;
//	bm0.BytesPerLine = xsize*bpp;
//	bm0.BitsPerPixel = bpp;
//	bm0.pData = p;
//	bm0.pPal = NULL;
//	bm0.pMethods = GUI_DRAW_BMPM565;
//	return p;
//}


#include "jinclude.h"
#include "jpeglib.h"

#include "bsp_ov7670.h"
#include "fsl_ov7670.h"
int bmp2jpeg_compress(unsigned char *inbuf, unsigned char **outbuf,unsigned long *outSize);
extern GUI_CONST_STORAGE GUI_BITMAP bm1;

extern uint32_t activeFrameAddr;
extern uint32_t inactiveFrameAddr;
extern camera_receiver_handle_t cameraReceiver;
unsigned long jpeg01_size=0;
uint8_t *jpeg01=0;
unsigned long jpeg02_size=0;
uint8_t *jpeg02=0;
uint8_t jpeg_idle=0;
uint8_t *rbg888=0;
void rgb2jpeg(uint16_t *p)
{	
	
	uint32_t j=0;
	
	for(int i;i<X_SIZE*Y_SIZE;i++)
	{
		rbg888[j++] = (p[i]&0xf800)>>11<<3;
		rbg888[j++] = (p[i]&0x7e0)>>5<<2;
		rbg888[j++] = (p[i]&0x1f)<<3;
//		rbg888[j++] = (p[i]&0x1f)<<3;
//		rbg888[j++] = (p[i]&0x7e0)>>5<<2;
//		rbg888[j++] = (p[i]&0xf800)<<3;
	}
	
	if((jpeg_idle&0x01)==0)
	{
		jpeg01_size = SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN);
		bmp2jpeg_compress(rbg888,&jpeg01,&jpeg01_size);
		jpeg_idle |= 0x01;
	}
	else if((jpeg_idle&0x02)==0)
	{
		jpeg02_size = SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN);
		bmp2jpeg_compress(rbg888,&jpeg02,&jpeg02_size);
		jpeg_idle |= 0x02;
	}
	else
	{
	
	}
	
}

uint32_t get_jpeg(uint8_t **p)
{
	if(jpeg_idle&0x01)
	{
		*p = jpeg01;
		return jpeg01_size;
	}
	else if(jpeg_idle&0x02)
	{
		*p = jpeg02;
		return jpeg02_size;
	}
	else
	{
		*p=NULL;
		return 0;
	}
}
void idle_jpeg(uint8_t *jpeg)
{
	if(jpeg01==jpeg)
	{
		jpeg_idle &= ~0x01;
	}
	else if(jpeg02==jpeg)
	{
		jpeg_idle &= ~0x02;
	}
}

__IO	uint32_t FPS_cnt=0;
void UI_app()
{
	
	GUI_SetBkColor(GUI_BLUE);
	GUI_SetColor(GUI_RED);
	GUI_Clear();
	
//	creatbmp(320,240,2);
//	memcpy((U8*)bm0.pData,bm1.pData,480*272*2);
//	GUI_DrawBitmap(&bm0,0,0);
	jpeg01 = staticMalloc(SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN));
	jpeg02 = staticMalloc(SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN));
	rbg888 = staticMalloc(SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN));
	bsp_InitCamera();
	
	rgb2jpeg((void*)activeFrameAddr);
  for(;;)
  {
			
		CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, activeFrameAddr);	
		activeFrameAddr = inactiveFrameAddr;
		/* Wait to get the full frame buffer to show. */
		while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &inactiveFrameAddr))
		{
		}	
		rgb2jpeg((void*)inactiveFrameAddr);
		FPS_cnt++;
	//	vTaskDelay(20);
  }
}




int bmp2jpeg_compress(unsigned char *inbuf, unsigned char **outbuf,unsigned long *outSize)
{
	int jpegWidth = 320;
	int jpegHeight = 240;
	struct jpeg_compress_struct toWriteCinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int row_stride;

	toWriteCinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&toWriteCinfo);

	jpeg_mem_dest(&toWriteCinfo, outbuf,outSize);

	toWriteCinfo.image_width = jpegWidth;
	toWriteCinfo.image_height = jpegHeight;
	toWriteCinfo.input_components = 3;
	toWriteCinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&toWriteCinfo);
	jpeg_set_quality(&toWriteCinfo, 75, TRUE);//0~100
	jpeg_start_compress(&toWriteCinfo,TRUE);
	row_stride = jpegWidth ;
	while(toWriteCinfo.next_scanline < toWriteCinfo.image_height)
	{
		row_pointer[0] = & inbuf[toWriteCinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&toWriteCinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&toWriteCinfo);
	jpeg_destroy_compress(&toWriteCinfo);
	return 0;
}
