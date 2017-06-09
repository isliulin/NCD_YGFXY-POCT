/***************************************************************************************************
*FileName: DeviceAdjustDao
*Description: ��д�豸У׼��¼
*Author: xsx_kair
*Data: 2017��6��2�� 15:53:25
***************************************************************************************************/

/***************************************************************************************************/
/******************************************Header List********************************************/
/***************************************************************************************************/
#include	"DeviceErrorDao.h"

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
MyState_TypeDef writeDeviceErrorToFile(DeviceError * deviceError)
{
	FatfsFileInfo_Def * myfile = NULL;
	DeviceRecordHeader * deviceRecordHeader = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(MyFileStructSize);
	deviceRecordHeader = MyMalloc(DeviceRecordHeaderStructSize);
	
	if(myfile && deviceError && deviceRecordHeader)
	{
		memset(myfile, 0, MyFileStructSize);
		memset(deviceRecordHeader, 0, DeviceRecordHeaderStructSize);
		
		myfile->res = f_open(&(myfile->file), DeviceErrorFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
			
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			f_lseek(&(myfile->file), 0);
			
			//��ȡ����ͷ
			myfile->res = f_read(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->br));
			if(deviceRecordHeader->crc != CalModbusCRC16Fun1(deviceRecordHeader, DeviceRecordHeaderStructCrcSize))
			{
				deviceRecordHeader->itemSize = 0;
				deviceRecordHeader->uploadIndex = 0;
				deviceRecordHeader->crc = CalModbusCRC16Fun1(deviceRecordHeader, DeviceRecordHeaderStructCrcSize);
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
			f_lseek(&(myfile->file), deviceRecordHeader->itemSize * DeviceErrorStructSize + DeviceRecordHeaderStructSize);
			myfile->res = f_write(&(myfile->file), deviceError, DeviceErrorStructSize, &(myfile->bw));

			if((FR_OK != myfile->res) || (myfile->bw != DeviceErrorStructSize))
				goto Finally;
			
			//��������ͷ
			deviceRecordHeader->itemSize += 1;
			deviceRecordHeader->crc = CalModbusCRC16Fun1(deviceRecordHeader, DeviceRecordHeaderStructCrcSize);
			
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

MyState_TypeDef readDeviceErrorFromFile(DeviceErrorReadPackge * deviceErrorReadPackge)
{
	FatfsFileInfo_Def * myfile = NULL;
	MyState_TypeDef statues = My_Fail;
	unsigned char  i = 0;
	
	myfile = MyMalloc(MyFileStructSize);

	if(myfile && deviceErrorReadPackge)
	{
		memset(myfile, 0, MyFileStructSize);
		memset(&(deviceErrorReadPackge->deviceRecordHeader), 0, DeviceRecordHeaderStructSize);
		memset(&(deviceErrorReadPackge->deviceError), 0, DeviceErrorStructSize * DeviceErrorRecordPageShowNum);
		deviceErrorReadPackge->readTotalNum = 0;
		
		myfile->res = f_open(&(myfile->file), DeviceErrorFileName, FA_READ);
		
		if(FR_OK == myfile->res)
		{
			myfile->size = f_size(&(myfile->file));
			
			f_lseek(&(myfile->file), 0);
			
			//��ȡ����ͷ
			myfile->res = f_read(&(myfile->file), &(deviceErrorReadPackge->deviceRecordHeader), DeviceRecordHeaderStructSize, &(myfile->br));
			if(deviceErrorReadPackge->deviceRecordHeader.crc != CalModbusCRC16Fun1(&(deviceErrorReadPackge->deviceRecordHeader), DeviceRecordHeaderStructCrcSize))
				goto Finally;
			
			//���pageRequest��crc���󣬱�ʾ�ǰ����ϴ�������ȡ���ݽ����ϴ�
			if(deviceErrorReadPackge->pageRequest.crc != CalModbusCRC16Fun1(&(deviceErrorReadPackge->pageRequest), PageRequestStructCrcSize))
			{
				if(deviceErrorReadPackge->deviceRecordHeader.uploadIndex < deviceErrorReadPackge->deviceRecordHeader.itemSize)
				{
					f_lseek(&(myfile->file), deviceErrorReadPackge->deviceRecordHeader.uploadIndex * DeviceErrorStructSize + DeviceRecordHeaderStructSize);
				
					if((deviceErrorReadPackge->deviceRecordHeader.itemSize - deviceErrorReadPackge->deviceRecordHeader.uploadIndex) >= DeviceErrorRecordPageShowNum)
						f_read(&(myfile->file), deviceErrorReadPackge->deviceError, DeviceErrorRecordPageShowNum * DeviceErrorStructSize, &(myfile->br));
					else
						f_read(&(myfile->file), deviceErrorReadPackge->deviceError, (deviceErrorReadPackge->deviceRecordHeader.itemSize - deviceErrorReadPackge->deviceRecordHeader.uploadIndex) * DeviceErrorStructSize, &(myfile->br));
				}
			}
			//���pageRequest��crc��ȷ����ʾ�ǰ���pageRequest���������ݽ��ж�ȡ����
			else if(deviceErrorReadPackge->pageRequest.startElementIndex < deviceErrorReadPackge->deviceRecordHeader.itemSize)
			{
				if(deviceErrorReadPackge->pageRequest.pageSize > (deviceErrorReadPackge->deviceRecordHeader.itemSize - deviceErrorReadPackge->pageRequest.startElementIndex))
					deviceErrorReadPackge->pageRequest.pageSize = (deviceErrorReadPackge->deviceRecordHeader.itemSize - deviceErrorReadPackge->pageRequest.startElementIndex);
					
				if(deviceErrorReadPackge->pageRequest.orderType == DESC)
					myfile->res = f_lseek(&(myfile->file), (deviceErrorReadPackge->pageRequest.startElementIndex) * DeviceErrorStructSize + DeviceRecordHeaderStructSize);
				else
					myfile->res = f_lseek(&(myfile->file), (deviceErrorReadPackge->deviceRecordHeader.itemSize - (deviceErrorReadPackge->pageRequest.pageSize + deviceErrorReadPackge->pageRequest.startElementIndex)) * DeviceErrorStructSize + DeviceRecordHeaderStructSize);
					
				myfile->res = f_read(&(myfile->file), deviceErrorReadPackge->deviceError, deviceErrorReadPackge->pageRequest.pageSize * DeviceErrorStructSize, &(myfile->br));	
				
				for(i=0; i<DeviceErrorRecordPageShowNum; i++)
				{
					if(deviceErrorReadPackge->deviceError[i].crc == CalModbusCRC16Fun1(&deviceErrorReadPackge->deviceError[i], DeviceErrorStructCrcSize))
						deviceErrorReadPackge->readTotalNum++;
				}
			}
			
			statues = My_Pass;
			
			Finally:
				f_close(&(myfile->file));
		}
	}
	MyFree(myfile);
	
	return statues;
}

MyState_TypeDef plusDeviceErrorHeaderUpLoadIndexToFile(unsigned int index)
{
	FatfsFileInfo_Def * myfile = NULL;
	DeviceRecordHeader * deviceRecordHeader = NULL;
	MyState_TypeDef statues = My_Fail;
	
	myfile = MyMalloc(MyFileStructSize);
	deviceRecordHeader = MyMalloc(DeviceRecordHeaderStructSize);

	if(myfile && deviceRecordHeader)
	{
		memset(myfile, 0, MyFileStructSize);

		myfile->res = f_open(&(myfile->file), DeviceErrorFileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
		
		if(FR_OK == myfile->res)
		{
			f_lseek(&(myfile->file), 0);

			myfile->res = f_read(&(myfile->file), deviceRecordHeader, DeviceRecordHeaderStructSize, &(myfile->br));
			if((FR_OK == myfile->res) && (DeviceRecordHeaderStructSize == myfile->br) 
				&& (deviceRecordHeader->crc == CalModbusCRC16Fun1(deviceRecordHeader, DeviceRecordHeaderStructCrcSize)))
			{
				deviceRecordHeader->uploadIndex += index;
				deviceRecordHeader->crc = CalModbusCRC16Fun1(deviceRecordHeader, DeviceRecordHeaderStructCrcSize);
				
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

MyState_TypeDef deleteDeviceErrorFile(void)
{
	FRESULT res;
	
	res = f_unlink(DeviceErrorFileName);
	
	if((FR_OK == res) || (FR_NO_FILE == res))
		return My_Pass;
	else
		return My_Fail;
}

/****************************************end of file************************************************/