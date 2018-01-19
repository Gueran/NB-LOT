#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
typedef struct _DeviceInfo
{
	char 	BaseName[64];//��վ
	char 	GUID[36];
	
	char 	Address[128];//λ��
	char    Switch[4];
	short 	BaseTemperature;//��׼�¶� 0.01��
	short   LedTemperature;//LED�¶�   ��Դ�¶�
	short	Brightness;    //����    �ٷֱ�
	short   ColorTemperature; //ɫ��    �ٷֱ�
	short   Voltage;//��ѹ  0.01V
	short   Current;//����0.01��A��
}DEVICEINFO;

void DeviceInfoInit();
void setDeviceInfoGUID(char * GUID);
void setDeviceInfoBaseTemperature(short BaseTemperature);
void setDeviceInfoLedTemperature(short LedTemperature);
void setDeviceInfoBrightness(short Brightness);
void setDeviceInfoColorTemperature(short ColorTemperature);
void setDeviceInfoVoltage(short Voltage);
void setDeviceInfoCurrent(short Current);
void DeviveInfotoJson(char * jsonbuff);


#endif
