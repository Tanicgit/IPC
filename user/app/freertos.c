#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "t_debug.h"
#include "bsp.h" 
#include "GUI.h"

#include "t_debug.h"
#include "t_shell.h"

#include	"lwip_app.h"
#include "main.h"
#include "GUI_VNC.h"
#include "app_ui.h"
BaseType_t xReturnedLed;
TaskHandle_t xHandleLed = NULL;
void vTaskCodeLed( void * pvParameters )
{
	for( ;; )
	{  
		vTaskDelay(1000);
	//	RGB_RED_LED_TOGGLE;
		Sys.dhcpSta=CheckDHCPsta();
	}
}
BaseType_t xReturnedShell;
TaskHandle_t xHandleShell = NULL;
shell_context_struct t_shell;
void vTaskCodeShell( void * pvParameters )
{
	SHELL_Init(&t_shell,t_send_data,t_recv_data,DbgConsole_Printf,"T#->");
	for( ;; )
	{  
		SHELL_Main(&t_shell);
	}
}
extern GUI_CONST_STORAGE GUI_BITMAP bm1;
extern GUI_CONST_STORAGE GUI_BITMAP bm2;
BaseType_t xReturnedUi;
TaskHandle_t xHandleUi = NULL;
void vTaskCodeUi(void * pvParameters)
{
	while(Sys.dhcpSta==0)
	{
		osDelay(200);
	}
	GUI_Init();
	
	GUI_VNC_X_StartServer(0,0);
	GUI_VNC_SetPassword((uint8_t*)"123456");
	GUI_VNC_SetProgName("tanic");
	GUI_VNC_SetSize(V_LCD_W,V_LCD_H);
	GUI_VNC_RingBell();
	
	UI_app();
  for(;;)
  {
  }
}

void freeRtosInit()
{
		LWIP_Init();//必须调度开始前初始化
	  xReturnedLed = xTaskCreate(vTaskCodeLed, "vTaskCodeLed",128, ( void * ) 1,1,&xHandleLed ); 
		xReturnedShell = xTaskCreate(vTaskCodeShell, "vTaskCodeShell",2048, ( void * ) 1,1,&xHandleShell ); 
		xReturnedUi = xTaskCreate(vTaskCodeUi, "vTaskCodeUi",2048, ( void * ) 1,1,&xHandleUi );
		
}
