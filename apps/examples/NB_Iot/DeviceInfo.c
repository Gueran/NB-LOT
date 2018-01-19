#include <stdio.h>
#include "cJSON.h"
#include "DeviceInfo.h"

static DEVICEINFO DeviceInfo;

/*
char	BaseName[64];//基站
char	GUID[32];

char	Address[128];//位置
short	BaseTemperature;//基准温度 0.01℃
short	LedTemperature;//LED温度	 光源温度
short	Brightness;    //亮度    百分比
short	ColorTemperature; //色温	  百分比
short	Voltage;//电压  0.01V
short	Current;//电流0.01（A）



*/
void DeviceInfoInit()
{
	memset(&DeviceInfo,0,sizeof(DeviceInfo));

	strcpy(DeviceInfo.BaseName, "TESTBase");
	strcpy(DeviceInfo.Address,	"TESAddressT");
	strcpy(DeviceInfo.GUID,	"B123456789012345678901234567890B");
	strcpy(DeviceInfo.Switch,	"OFF");
}
void setDeviceInfoGUID(char * GUID)
{
	strcpy(DeviceInfo.GUID , GUID);
}

void setDeviceInfoBaseTemperature(short BaseTemperature)
{
	DeviceInfo.BaseTemperature = BaseTemperature;
}
void setDeviceInfoLedTemperature(short LedTemperature)
{
	DeviceInfo.LedTemperature = LedTemperature;
}
void setDeviceInfoBrightness(short Brightness)
{
	DeviceInfo.Brightness = Brightness;
}
void setDeviceInfoColorTemperature(short ColorTemperature)
{
	DeviceInfo.ColorTemperature = ColorTemperature;
}
void setDeviceInfoVoltage(short Voltage)
{
	DeviceInfo.Voltage = Voltage;
}
void setDeviceInfoCurrent(short Current)
{
	DeviceInfo.Current = Current;
}


void DeviveInfotoJson(char * jsonbuff)
{
	// Create JSON Object 
	char *buf ;
    cJSON *json = cJSON_CreateObject();
	cJSON *object = NULL;

	cJSON_AddItemToObject(json,"DeviceInfo",object = cJSON_CreateObject());

	cJSON_AddStringToObject(object,"BaseName",DeviceInfo.BaseName);
	cJSON_AddStringToObject(object,"GUID",DeviceInfo.GUID);
	cJSON_AddStringToObject(object,"Address",DeviceInfo.Address);
	cJSON_AddStringToObject(object,"Switch",DeviceInfo.Switch);
	cJSON_AddNumberToObject(object,"BaseTemperature",DeviceInfo.BaseTemperature);
	cJSON_AddNumberToObject(object,"LedTemperature",DeviceInfo.LedTemperature);
	cJSON_AddNumberToObject(object,"Brightness",DeviceInfo.Brightness);
	cJSON_AddNumberToObject(object,"ColorTemperature",DeviceInfo.ColorTemperature);
	cJSON_AddNumberToObject(object,"Voltage",DeviceInfo.Voltage);
	cJSON_AddNumberToObject(object,"Current",DeviceInfo.Current);

	//
    
    buf = cJSON_PrintUnformatted(json);
	strcpy(jsonbuff,buf);
	printf("%s\nlen:%d\n",jsonbuff,strlen(jsonbuff));
	cJSON_Delete(json);
}




