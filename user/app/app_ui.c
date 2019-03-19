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
unsigned long jpeg_size=0;
uint8_t *jpeg=0;
uint8_t jpeg_sta=0;
uint8_t *rbg888=0;
void rgb2jpeg(uint16_t *p)
{	
	
	uint32_t j=0;	

	if(jpeg_sta==1)
	{
		jpeg_size = SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN);
//		memset(rbg888,0,jpeg_size);
//		for(int y=0;y<240;y++)
//		{
//			for(int x=0;x<320;x++)
//			{
//				if(x>=10&&x<=60)
//				{
//					rbg888[j++] = 0xff;
//					rbg888[j++] = 0x00;
//					rbg888[j++] = 0x00;
//				}			
//				else if(x>=100&&x<=160)
//				{
//					rbg888[j++] = 0x00;
//					rbg888[j++] = 0xff;
//					rbg888[j++] = 0x00;
//				}
//				
//				else if(x>=200&&x<=260)
//				{
//					rbg888[j++] = 0x00;
//					rbg888[j++] = 0x00;
//					rbg888[j++] = 0xff;
//				}
//				else
//				{
//					j+=3;
//				}
//				
//			}
//		}
		
		for(int i=0;i<X_SIZE*Y_SIZE;i++)
		{
			rbg888[j++] = ((p[i]&0xf800)>>11)<<3;
			rbg888[j++] = ((p[i]&0x7e0)>>5)<<2;
			rbg888[j++] = (p[i]&0x1f)<<3;
	//		rbg888[j++] = (p[i]&0x1f)<<3;
	//		rbg888[j++] = (p[i]&0x7e0)>>5<<2;
	//		rbg888[j++] = (p[i]&0xf800)<<3;
			
			
		}
		
		bmp2jpeg_compress(rbg888,&jpeg,&jpeg_size);
		jpeg_sta = 0;
	}
}

uint32_t get_jpeg(uint8_t **p)
{
	if(jpeg_sta==0)
	{
		jpeg_sta=1;
		while(jpeg_sta)
		{
			osDelay(10);
		}
		*p = jpeg;
		return jpeg_size;
	}
	else
	{
		*p=NULL;
		return 0;
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
	jpeg = staticMalloc(SDK_SIZEALIGN(X_SIZE*Y_SIZE*3,FRAME_BUFFER_ALIGN));
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
	toWriteCinfo.input_components = 3;//3
	toWriteCinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&toWriteCinfo);
	jpeg_set_quality(&toWriteCinfo, 75, TRUE);//0~100
	jpeg_start_compress(&toWriteCinfo,TRUE);
	row_stride = jpegWidth ;
	while(toWriteCinfo.next_scanline < toWriteCinfo.image_height)
	{
		row_pointer[0] = & inbuf[toWriteCinfo.next_scanline * row_stride * 3];//ÐÐÊ×Æ«ÒÆµØÖ·
		(void)jpeg_write_scanlines(&toWriteCinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&toWriteCinfo);
	jpeg_destroy_compress(&toWriteCinfo);
	return 0;
}
