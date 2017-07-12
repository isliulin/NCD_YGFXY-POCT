/***************************************************************************************************
*FileName: TestDataDao
*Description: ��������dao
*Author: xsx_kair
*Data: 2016��12��8��10:43:26
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/
#include	"RecordDataDao.h"

#include	"CRC16.h"
#include	"MyMem.h"

#include	"ff.h"

#include	<string.h>
#include	"stdio.h"
#include 	"stdlib.h"
/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/


/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
/****************************************File Start*************************************************/
/***************************************************************************************************/
/***************************************************************************************************/

/***************************************************************************************************
*FunctionName:  writeRecordDataToFile
*Description:  �����¼��sd�����˴�Ϊͨ��д������ֻ����д���ݺ͸�������ͷ������ �������ݵ�crc��Ҫ�ڴ�֮ǰ�������
*Input: 	fileName -- д�ļ���
*			recordData -- д������
*			recordItemSize -- д�����ݵ�Ԫ����
*Output:  
*Return:  
*Author:  xsx
*Date: 2017��7��3�� 15:42:09
***************************************************************************************************/
MyRes writeRecordDataToFile(const char * fileName, void * recordData, unsigned int recordItemSize)
{
	FatfsFileInfo_Def * myfile = NULL;
	DeviceRecordHeader * deviceRecordHeader = NULL;
	MyRes statues = My_Fail;
	
	myfile = MyMalloc(MyFileStructSize);
	deviceRecordHeader = MyMalloc(DeviceRecordHeaderStructSize);
	
	if(myfile && recordData && deviceRecordHeader && fileName)
	{
		memset(myfile, 0, MyFileStructSize);
		memset(deviceRecordHeader, 0, DeviceRecordHeaderStructSize);
		
		myfile->res = f_open(&(myfile->file), fileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			f_lseek(&(myfile->file), 0);
			
			//��ȡ����ͷ
			myfile->res = f_read(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->br));
			if(deviceRecordHeader->crc != CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL))
			{
				deviceRecordHeader->itemSize = 0;
				deviceRecordHeader->userUpLoadIndex = 0;
				deviceRecordHeader->ncdUpLoadIndex = 0;
				deviceRecordHeader->crc = CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL);
			}
			
			//��дһ������ͷ����ֹû������ͷд�����쳣
			if(0 == myfile->size)
			{
				f_lseek(&(myfile->file), 0);
				myfile->res = f_write(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->bw));
				if((FR_OK != myfile->res) || (myfile->bw != DeviceRecordHeaderStructSize))
					goto Finally;
			}
			
			//д����
			f_lseek(&(myfile->file), deviceRecordHeader->itemSize * recordItemSize + DeviceRecordHeaderStructSize);
			myfile->res = f_write(&(myfile->file), recordData, recordItemSize, &(myfile->bw));

			if((FR_OK != myfile->res) || (myfile->bw != recordItemSize))
				goto Finally;
			
			//��������ͷ
			deviceRecordHeader->itemSize += 1;
			deviceRecordHeader->crc = CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL);
			
			f_lseek(&(myfile->file), 0);
			myfile->res = f_write(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->bw));
			if((FR_OK == myfile->res) && (myfile->bw == DeviceRecordHeaderStructSize))
				statues = My_Pass;
			
			Finally:
				f_close(&(myfile->file));
		}
	}
	
	MyFree(myfile);
	MyFree(deviceRecordHeader);
	
	return statues;
}

/***************************************************************************************************
*FunctionName:  readRecordDataFromFile
*Description:  ��sd��ȡ��¼���ݣ��������pageRequest��ȡ�����ȡ���ݱ�����recordData1��
*	����recordData1�б�������û��ϴ�������ȡ�����ݣ�recordData2�������Ŧ�����ϴ�������ȡ������
*Input:  	fileName -- �ļ���
*			pageRequest -- ��ҳ�������
*			deviceRecordHeader -- ��ȡ����ͷ��ַ
*			recordData1 -- �����ȡ�����ݵ�ַ1
*			recordData2 -- �����ȡ�����ݵ�ַ2
*			recordItemSize -- ��ȡ������Ŀ��С
*Output:  
*Return:  
*Author:  xsx
*Date: 2017��7��3�� 16:14:55
***************************************************************************************************/
MyRes readRecordDataFromFile(const char * fileName, PageRequest * pageRequest, DeviceRecordHeader * deviceRecordHeader, 
	void * recordData1, void * recordData2, unsigned short recordItemSize)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyRes statues = My_Fail;
	
	myfile = MyMalloc(MyFileStructSize);

	if(myfile && fileName && deviceRecordHeader)
	{
		memset(myfile, 0, MyFileStructSize);
		memset(deviceRecordHeader, 0, DeviceRecordHeaderStructSize);
		
		myfile->res = f_open(&(myfile->file), fileName, FA_READ);
		
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			
			f_lseek(&(myfile->file), 0);
			
			//��ȡ����ͷ
			myfile->res = f_read(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->br));
			if(deviceRecordHeader->crc != CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL))
				goto Finally;
			
			//���pageRequestΪ�գ�������ϴ�������ȡ����
			if(pageRequest == NULL)
			{
				//�����û��ϴ�������ȡ����
				if(deviceRecordHeader->userUpLoadIndex < deviceRecordHeader->itemSize)
				{
					f_lseek(&(myfile->file), deviceRecordHeader->userUpLoadIndex * recordItemSize + DeviceRecordHeaderStructSize);
				
					f_read(&(myfile->file), recordData1, recordItemSize, &(myfile->br));
				}
				
				//����Ŧ�����ϴ�������ȡ����
				if(deviceRecordHeader->ncdUpLoadIndex < deviceRecordHeader->itemSize)
				{
					f_lseek(&(myfile->file), deviceRecordHeader->ncdUpLoadIndex * recordItemSize + DeviceRecordHeaderStructSize);
				
					f_read(&(myfile->file), recordData2, recordItemSize, &(myfile->br));
				}
			}
			//���pageRequest != NULL����ʾ�ǰ���pageRequest���������ݽ��ж�ȡ����
			else if(pageRequest->startElementIndex < deviceRecordHeader->itemSize)
			{
				if(pageRequest->pageSize > (deviceRecordHeader->itemSize - pageRequest->startElementIndex))
					pageRequest->pageSize = (deviceRecordHeader->itemSize - pageRequest->startElementIndex);
					
				if(pageRequest->orderType == DESC)
					myfile->res = f_lseek(&(myfile->file), (pageRequest->startElementIndex) * recordItemSize + DeviceRecordHeaderStructSize);
				else
					myfile->res = f_lseek(&(myfile->file), (deviceRecordHeader->itemSize - (pageRequest->pageSize + pageRequest->startElementIndex)) * recordItemSize + DeviceRecordHeaderStructSize);
					
				f_read(&(myfile->file), recordData1, pageRequest->pageSize * recordItemSize, &(myfile->br));	
			}
			
			statues = My_Pass;
			
			Finally:
				f_close(&(myfile->file));
		}
	}
	MyFree(myfile);
	
	return statues;
}

/***************************************************************************************************
*FunctionName:  plusRecordDataHeaderUpLoadIndexToFile
*Description:  �����ϴ���������ԭ�����ϼ���һ������
*Input:  	fileName -- �ļ���
*			userIndexPlusNum -- �û��ϴ������ӵĴ�С
*			ncdIndexPlusNum -- Ŧ�����ϴ������ӵĴ�С
*Output:  
*Return:  
*Author:  xsx
*Date: 2017��7��3�� 16:16:44
***************************************************************************************************/
MyRes plusRecordDataHeaderUpLoadIndexToFile(const char * fileName, unsigned char userIndexPlusNum, unsigned char ncdIndexPlusNum)
{
	FatfsFileInfo_Def * myfile = NULL;
	DeviceRecordHeader * deviceRecordHeader = NULL;
	MyRes statues = My_Fail;
	
	myfile = MyMalloc(MyFileStructSize);
	deviceRecordHeader = MyMalloc(DeviceRecordHeaderStructSize);

	if(myfile && deviceRecordHeader)
	{
		memset(myfile, 0, MyFileStructSize);

		myfile->res = f_open(&(myfile->file), fileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);

			myfile->res = f_read(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->br));
			if((FR_OK == myfile->res) && (DeviceRecordHeaderStructSize == myfile->br) 
				&& (deviceRecordHeader->crc == CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL)))
			{
				deviceRecordHeader->userUpLoadIndex += userIndexPlusNum;
				deviceRecordHeader->ncdUpLoadIndex += ncdIndexPlusNum;
				deviceRecordHeader->crc = CalModbusCRC16Fun(deviceRecordHeader, DeviceRecordHeaderStructCrcSize, NULL);
				
				f_lseek(&(myfile->file), 0);
				myfile->res = f_write(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->bw));
				if((FR_OK == myfile->res)&&(myfile->bw == DeviceRecordHeaderStructSize))
					statues = My_Pass;
			}
			
			f_close(&(myfile->file));
		}
	}
	MyFree(myfile);
	MyFree(deviceRecordHeader);
	
	return statues;
}

/***************************************************************************************************
*FunctionName:  deleteRecordDataFile
*Description:  ɾ�����ݼ�¼�ļ�
*Input:  	fileName -- ɾ�����ļ���
*Output:  
*Return:  
*Author:  xsx
*Date: 2017��7��3�� 16:18:06
***************************************************************************************************/
MyRes deleteRecordDataFile(const char * fileName)
{
	FRESULT res;
	
	res = f_unlink(fileName);
	
	if((FR_OK == res) || (FR_NO_FILE == res))
		return My_Pass;
	else
		return My_Fail;
}
/****************************************end of file************************************************/