/***************************************************************************************************
*FileName:
*Description:
*Author:
*Data:
***************************************************************************************************/

/***************************************************************************************************/
/******************************************ͷ�ļ�***************************************************/
/***************************************************************************************************/
#include	"HttpPostTool.h"
#include	"QueueUnits.h"
#include	"System_Data.h"
#include 	"Usart4_Driver.h"
#include	"SystemSet_Data.h"
#include	"AppFileDao.h"
#include	"IAP_Fun.h"
#include	"RemoteSoft_Data.h"
#include	"WifiFunction.h"
#include	"MyMem.h"

#include	"FreeRTOS.h"
#include 	"task.h"
#include 	"queue.h"
#include	"semphr.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"
/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/
static void PostDataByWireNet(HttpPostBuffer * httpPostBuffer);
static void PostDataByWifi(HttpPostBuffer * httpPostBuffer);
/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************����********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/


void PostData(HttpPostBuffer * httpPostBuffer)
{
	if(httpPostBuffer)
	{
		memset(httpPostBuffer->recvBuf, 0, SERVERRECVBUFLEN);
		PostDataByWireNet(httpPostBuffer);
		if(httpPostBuffer)
	}
}

static void PostDataByWireNet(HttpPostBuffer * httpPostBuffer)
{

}

static void PostDataByWifi(HttpPostBuffer * httpPostBuffer)
{

}

static bool receiveBufferLenIsGreaterThanZero(HttpPostBuffer * httpPostBuffer)
{
	
}