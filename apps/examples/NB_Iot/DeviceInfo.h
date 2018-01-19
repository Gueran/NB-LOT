#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
typedef struct _DeviceInfo
{
	char 	BaseName[64];//基站
	char 	GUID[36];
	
	char 	Address[128];//位置
	char    Switch[4];
	short 	BaseTemperature;//基准温度 0.01℃
	short   LedTemperature;//LED温度   光源温度
	short	Brightness;    //亮度    百分比
	short   ColorTemperature; //色温    百分比
	short   Voltage;//电压  0.01V
	short   Current;//电流0.01（A）
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
