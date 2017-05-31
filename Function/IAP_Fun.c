/***************************************************************************************************
*FileName:IAP_Fun
*Description:IAP����
*Author:xsx
*Data:2017��2��16��16:27:25
***************************************************************************************************/


/***************************************************************************************************/
/******************************************ͷ�ļ�***************************************************/
/***************************************************************************************************/
#include	"IAP_Fun.h"

#include	"RemoteSoftDao.h"
#include	"AppFileDao.h"

#include	"RemoteSoft_Data.h"

#include	"MyMem.h"
#include	"Md5.h"
#include	"Define.h"
#include	"MyTools.h"
#include	"Delay.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"

/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/**************************************�ֲ���������*************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************����********************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName: checkMd5
*Description: ���㲢�Ա�MD5
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2017��2��20��14:02:35
***************************************************************************************************/
MyState_TypeDef checkMd5(void)
{
	RemoteSoftInfo * remoteSoftInfo = NULL;		//��ȡ�Ĺ̼���Ϣ
	char currentMd5[40];				//��ǰMD5
	
	remoteSoftInfo = MyMalloc(sizeof(RemoteSoftInfo));
	if(remoteSoftInfo)
	{
		//��ȡ�ļ��е�md5
		memset(remoteSoftInfo, 0, sizeof(RemoteSoftInfo));
		if(My_Fail == ReadRemoteSoftInfo(remoteSoftInfo))
			return My_Fail;
	}
	else
		return My_Fail;
	
	
	//������¹̼���md5ֵ
	memset(currentMd5, 0, 40);
	md5sum(currentMd5);
	
	//�Ա�MD5
	if(true == CheckStrIsSame(remoteSoftInfo->md5, currentMd5, 32))
		return My_Pass;
	else
		return My_Fail;
}

/***************************************************************************************************
*FunctionName: checkNewFirmwareIsSuccessDownload
*Description: ����Ƿ�ɹ������¹̼�
*Input: 
*Output: 
*Return: 
*Author: xsx
*Date: 2017��2��21��09:02:28
***************************************************************************************************/
MyState_TypeDef checkNewFirmwareIsSuccessDownload(void)
{
	//����Ƿ����³���
	if(My_Pass == checkNewAppFileIsExist())
	{
		//������¹̼����Ա�MD5
		if(My_Pass == checkMd5())
			return My_Pass;	
	}
	
	return My_Fail;
}

