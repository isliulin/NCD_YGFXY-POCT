/***************************************************************************************************
*FileName:
*Description:
*Author:xsx
*Data:
***************************************************************************************************/


/***************************************************************************************************/
/******************************************ͷ�ļ�***************************************************/
/***************************************************************************************************/

#include	"HttpClient_Task.h"
#include	"UpLoad_Fun.h"

#include 	"FreeRTOS.h"
#include 	"task.h"


/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/
#define vHttpClientTask_PRIORITY			( ( unsigned portBASE_TYPE ) 2U )
const char * HttpClientTaskName = "vHttpClientTask";


/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/

static void vHttpClientTask( void *pvParameters );





/***************************************************************************************************/
/***************************************************************************************************/
/***************************************����********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

char StartvHttpClientTask(void)
{
	return xTaskCreate( vHttpClientTask, HttpClientTaskName, configMINIMAL_STACK_SIZE*2, NULL, vHttpClientTask_PRIORITY, NULL );
}


static void vHttpClientTask( void *pvParameters )
{

	while(1)
	{
		UpLoadFunction();
	}
}