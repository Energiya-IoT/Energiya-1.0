#include "legato.h"

#include "interfaces.h"
#include "ServiceOctaveGPIO.h"

/*
 * ServiceOctave.c
 *
 *  Created on: Oct 3, 2019
 *     Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *         Tel: +48 505 148 375
 */

void DeleteMainResources(uint8_t IoTType)
{
	LE_INFO("Clear Main Resources.");
	if(IoTType == 0)
	{
		io_DeleteResource("settings/addon_type");
	}

	io_DeleteResource("settings/i2c");
	io_DeleteResource("settings/i2c_address");
	io_DeleteResource("settings/i2c_high_resolution");

	io_DeleteResource("uc_input1/value");
	io_DeleteResource("uc_input1/enable");
	io_DeleteResource("uc_input1/period");
	io_DeleteResource("uc_input1/trigger");
	io_DeleteResource("uc_input1/interface");

	io_DeleteResource("uc_input2/value");
	io_DeleteResource("uc_input2/enable");
	io_DeleteResource("uc_input2/period");
	io_DeleteResource("uc_input2/trigger");
	io_DeleteResource("uc_input2/interface");

	io_DeleteResource("uc_input3/value");
	io_DeleteResource("uc_input3/enable");
	io_DeleteResource("uc_input3/period");
	io_DeleteResource("uc_input3/trigger");
	io_DeleteResource("uc_input3/interface");

	io_DeleteResource("uc_input4/value");
	io_DeleteResource("uc_input4/enable");
	io_DeleteResource("uc_input4/period");
	io_DeleteResource("uc_input4/trigger");
	io_DeleteResource("uc_input4/interface");
}

void DeleteAddOnResources()
{
	LE_INFO("Clear AddOn Resources.");
	if(GPIOS[0].IoHandlerRef != NULL)
	{
		LE_INFO("Clear IO Handler 1");
		io_RemoveBooleanPushHandler((io_BooleanPushHandlerRef_t)GPIOS[0].IoHandlerRef);
		GPIOS[0].IoHandlerRef = NULL;
	}
	if(GPIOS[1].IoHandlerRef != NULL)
	{
		LE_INFO("Clear IO Handler 2");
		io_RemoveBooleanPushHandler((io_BooleanPushHandlerRef_t)GPIOS[1].IoHandlerRef);
		GPIOS[1].IoHandlerRef = NULL;
	}
	if(GPIOS[2].IoHandlerRef != NULL)
	{
		LE_INFO("Clear IO Handler 3");
		io_RemoveBooleanPushHandler((io_BooleanPushHandlerRef_t)GPIOS[2].IoHandlerRef);
		GPIOS[2].IoHandlerRef = NULL;
	}
	if(GPIOS[3].IoHandlerRef != NULL)
	{
		LE_INFO("Clear IO Handler 4");
		io_RemoveBooleanPushHandler((io_BooleanPushHandlerRef_t)GPIOS[3].IoHandlerRef);
		GPIOS[3].IoHandlerRef = NULL;
	}

	if(GPIOS[0].EventRef != NULL)
	{
		LE_INFO("Clear IO Handler 1");
		le_gpioPin42_RemoveChangeEventHandler((le_gpioPin42_ChangeEventHandlerRef_t)GPIOS[0].EventRef);
		GPIOS[0].EventRef = NULL;
	}
	if(GPIOS[1].EventRef != NULL)
	{
		LE_INFO("Clear IO Handler 2");
		le_gpioPin33_RemoveChangeEventHandler((le_gpioPin33_ChangeEventHandlerRef_t)GPIOS[1].EventRef);
		GPIOS[1].EventRef = NULL;
	}
	if(GPIOS[2].EventRef != NULL)
	{
		LE_INFO("Clear IO Handler 3");
		le_gpioPin13_RemoveChangeEventHandler((le_gpioPin13_ChangeEventHandlerRef_t)GPIOS[2].EventRef);
		GPIOS[2].EventRef = NULL;
	}
	if(GPIOS[3].EventRef != NULL)
	{
		LE_INFO("Clear IO Handler 4");
		le_gpioPin8_RemoveChangeEventHandler((le_gpioPin8_ChangeEventHandlerRef_t)GPIOS[3].EventRef);
		GPIOS[3].EventRef = NULL;
	}
	if(AddressHandlerGPIO_I2C != NULL)
	{
		LE_INFO("Clear IO Address Handler GPIO I2C");
		io_RemoveNumericPushHandler(AddressHandlerGPIO_I2C);
		AddressHandlerGPIO_I2C = NULL;
	}

	io_DeleteResource("addon_gpio/input1");
	io_DeleteResource("addon_gpio/input2");
	io_DeleteResource("addon_gpio/input3");
	io_DeleteResource("addon_gpio/input4");
	io_DeleteResource("addon_gpio/input5");
	io_DeleteResource("addon_gpio/input6");
	io_DeleteResource("addon_gpio/input7");
	io_DeleteResource("addon_gpio/input8");
	io_DeleteResource("addon_gpio/i2c_address");

	io_DeleteResource("addon_gpio/output1");
	io_DeleteResource("addon_gpio/output2");
	io_DeleteResource("addon_gpio/output3");
	io_DeleteResource("addon_gpio/output4");

	le_gpioPin8_DisableEdgeSense();
	le_gpioPin13_DisableEdgeSense();
	le_gpioPin33_DisableEdgeSense();
	le_gpioPin42_DisableEdgeSense();
	/*le_gpioPin8_DisableResistors();
	le_gpioPin13_DisableResistors();
	le_gpioPin33_DisableResistors();
	le_gpioPin24_DisableResistors();
	le_gpioPin8_DisconnectService ();
	le_gpioPin33_DisconnectService ();
	le_gpioPin13_DisconnectService ();
	le_gpioPin24_DisconnectService ();*/
}

struct Input
{
    bool Enabled;
    double Interval;
    le_timer_Ref_t TimerRef;
    char Path[50];
    le_ExtAdc_IOT_ADC_t Sensor;
    le_ExtAdc_InterfaceType_t Interface;
}
Input1, Input2, Input3, Input4;

struct SettingsI2C
{
    char I2C[50];
    le_ExtAdc_Pins_t Address;
    bool HighResolution;
    uint8_t AddOnType;
}
Settings;

static void UpdateData(struct Input* I)
{
	uint16_t RawData;
	double Percentage;
	double ConvertedValue;
	le_ExtAdc_GetValue(I->Sensor, &RawData, &Percentage, &ConvertedValue);
	io_PushNumeric(I->Path, IO_NOW, Percentage);
	LE_INFO("Value Pushed.");
}

static void HandleSettingsI2CPush(double timestamp, const char* Str, void* contextPtr)
{
	struct SettingsI2C* I = contextPtr;
	strcpy(I->I2C, Str);
	le_ExtAdc_SetFileDescriptor(Str);
	LE_INFO(Str);
}

static void HandleSettingsAddressPush(double timestamp, double Address, void* contextPtr)
{
	struct SettingsI2C* I = contextPtr;
	uint8_t Addr = (uint8_t)Address;
	I->Address = (le_ExtAdc_Pins_t)Addr;
	le_ExtAdc_SetDeviceAddress(I->Address);
}

void Init1_I();
void Init2_I();
void Init1_O();
void Init2_O();
void Init_8Inputs();

static void HandleSettingsAddOnPush(double timestamp, double AddOnType, void* contextPtr)
{
	struct SettingsI2C* I = contextPtr;
	uint8_t AddOn = (uint8_t)AddOnType;
	I->AddOnType = AddOn;
	le_gpioPin42_TryConnectService();
	le_gpioPin33_TryConnectService();
	le_gpioPin13_TryConnectService();
	le_gpioPin8_TryConnectService();
	DeleteAddOnResources();
	switch(I->AddOnType)
	{
		case 1:
			LE_INFO("Outputs ON.");
			Init1_I();
			Init2_I();
			break;
		case 2:
			LE_INFO("Inputs ON.");
			Init1_O();
			Init2_O();
			break;
		case 3:
			LE_INFO("Outputs/Inputs ON.");
			Init1_O();
			Init2_I();
			break;
		case 4:
			LE_INFO("8 Inputs ON.");
			Init_8Inputs();
			break;
	}
}

static void HandleInterfacePush(double timestamp, double Interface, void* contextPtr)
{
	struct Input* I = contextPtr;
	uint8_t Inter = (uint8_t)Interface;
	I->Interface = (le_ExtAdc_InterfaceType_t)Inter;
	le_ExtAdc_SetInterface(I->Sensor, I->Interface);
}

static void HandleSettingsHighResolutionPush(double timestamp, bool High, void* contextPtr)
{
	struct SettingsI2C* I = contextPtr;
	I->HighResolution = High;
	if(High)
	{
		le_ExtAdc_SetConversionSpeed(LE_EXTADC_HIGHPRECISION15SPS);
		LE_INFO("High precision");
	}
	else
	{
		le_ExtAdc_SetConversionSpeed(LE_EXTADC_LOWPRECISION240SPS);
		LE_INFO("Low precision");
	}
}

static void Timer(le_timer_Ref_t timer)
{
	if(Input1.TimerRef == timer)
	{
		UpdateData(&Input1);
	}
	else if(Input2.TimerRef == timer)
	{
		UpdateData(&Input2);
	}
	else if(Input3.TimerRef == timer)
	{
		UpdateData(&Input3);
	}
	else if(Input4.TimerRef == timer)
	{
		UpdateData(&Input4);
	}

}

static void HandleEnablePush(double timestamp, bool enable, void* contextPtr)
{
	struct Input* I = contextPtr;
	I->Enabled = enable;
    if (enable)
    {
        if (I->Interval > 0.0)
        {
            le_timer_Start(I->TimerRef);
        }
    }
    else
    {
        le_timer_Stop(I->TimerRef);
    }
}

static void HandleIntervalPush(double timestamp, double period, void* contextPtr)
{
	struct Input* I = contextPtr;

    if (I->Interval != period)
    {
        if ((period <= 0.0) || (period > (double)(0x7FFFFFFF)))
        {
            le_timer_Stop(I->TimerRef);
            I->Interval = 0.0;
        }
        else
        {
            le_timer_SetMsInterval(I->TimerRef, period * 1000);

            if ((I->Interval == 0) && (I->Enabled))
            {
                le_timer_Start(I->TimerRef);
            }

            I->Interval = period;
        }
    }
}

static void HandleTriggerPush(double timestamp, void* contextPtr)
{
    struct Input* I = contextPtr;

    if (I->Enabled)
    {
    	UpdateData(I);
    	LE_INFO("Trigger fired.");
    }
}

void Init(uint8_t IoTType)
{
	char* No;
	struct Input* I;

	char SettingsDest1[50];
	char* SettingsDestConst1 = "settings";
	strcpy(SettingsDest1, SettingsDestConst1);
	strcat(SettingsDest1, "/addon_type");
	le_result_t  Res = io_CreateOutput(SettingsDest1, IO_DATA_TYPE_NUMERIC, "");
    if (Res != LE_OK)
    {
        LE_INFO("Failed to create Settings AddOn Type");
    }
	else
	{
    	io_SetNumericDefault(SettingsDest1, 0);
	}
	io_AddNumericPushHandler(SettingsDest1, HandleSettingsAddOnPush, &Settings);

	if(IoTType == 2)
	{
		char SettingsDest2[50];
		char* SettingsDestConst2 = "settings";
		strcpy(SettingsDest2, SettingsDestConst2);
		strcat(SettingsDest2, "/i2c_address");
		Res = io_CreateOutput(SettingsDest2, IO_DATA_TYPE_NUMERIC, "");
		if (Res != LE_OK)
		{
			LE_INFO("Failed to create Settings I2C Address");
		}
		else
		{
			io_SetNumericDefault(SettingsDest2, 0);
		}
		io_AddNumericPushHandler(SettingsDest2, HandleSettingsAddressPush, &Settings);

		//io_DeleteResource("UC_HighResolution/i2c_high_resolution");
		char SettingsDest3[50];
		char* SettingsDestConst3 = "settings";
		strcpy(SettingsDest3, SettingsDestConst3);
		strcat(SettingsDest3, "/i2c_high_resolution");
		Res = io_CreateOutput(SettingsDest3, IO_DATA_TYPE_BOOLEAN, "");
		if (Res != LE_OK)
		{
			LE_INFO("Failed to create Settings Resolution");
		}
		else
		{
			io_SetBooleanDefault(SettingsDest3, true);
		}
		io_AddBooleanPushHandler(SettingsDest3, HandleSettingsHighResolutionPush, &Settings);

		char SettingsDest4[50];
		char* SettingsDestConst4 = "settings";
		strcpy(SettingsDest4, SettingsDestConst4);
		strcat(SettingsDest4, "/i2c");
		Res = io_CreateOutput(SettingsDest4, IO_DATA_TYPE_STRING, "");
		if (Res != LE_OK)
		{
			LE_INFO("Failed to create Settings I2C");
		}
		else
		{
			io_SetStringDefault(SettingsDest4, "/dev/i2c-4");
		}
		io_AddStringPushHandler(SettingsDest4, HandleSettingsI2CPush, &Settings);

		for(int i = 0; i < 4; i++)
		{
			switch(i)
			{
				case 0:
					I = &Input1;
					I->Sensor = LE_EXTADC_IOT_ADC1;
					No = "1";
					break;
				case 1:
					I = &Input2;
					I->Sensor = LE_EXTADC_IOT_ADC2;
					No = "2";
					break;
				case 2:
					I = &Input3;
					I->Sensor = LE_EXTADC_IOT_ADC3;
					No = "3";
					break;
				case 3:
					I = &Input4;
					I->Sensor = LE_EXTADC_IOT_ADC4;
					No = "4";
					break;
			}

			char Dest1[50];
			char* UC1 = "uc_input";
			strcpy(Dest1, UC1);
			strcat(Dest1, No);
			I->TimerRef = le_timer_Create(Dest1);
			le_timer_SetRepeat(I->TimerRef, 0);
			le_timer_SetHandler(I->TimerRef, Timer);
			strcat(Dest1, "/value");
			strcpy(I->Path, Dest1);
			Res = io_CreateInput(Dest1, IO_DATA_TYPE_NUMERIC, "percent");
			if (Res != LE_OK)
			{
				LE_INFO("Failed to create Data Hub Input %s", No);
			}

			char Dest2[50];
			strcpy(Dest2, UC1);
			strcat(Dest2, No);
			strcat(Dest2, "/enable");
			Res = io_CreateOutput(Dest2, IO_DATA_TYPE_BOOLEAN, "");
			if (Res != LE_OK)
			{
				LE_INFO("Failed to create Data Hub Output %s", No);
			}
			else
			{
				io_SetBooleanDefault(Dest2, false);
			}
			io_AddBooleanPushHandler(Dest2, HandleEnablePush, I);

			char Dest3[50];
			strcpy(Dest3, UC1);
			strcat(Dest3, No);
			strcat(Dest3, "/period");
			Res = io_CreateOutput(Dest3, IO_DATA_TYPE_NUMERIC, "s");
			if (Res != LE_OK)
			{
				LE_INFO(Dest3);
				LE_INFO("Failed to create Data Hub Output Numeric %s", No);
			}
			else
			{
				io_SetNumericDefault(Dest3, 0);
			}
			io_AddNumericPushHandler(Dest3, HandleIntervalPush, I);

			char Dest4[50];
			strcpy(Dest4, UC1);
			strcat(Dest4, No);
			strcat(Dest4, "/trigger");
			Res = io_CreateOutput(Dest4, IO_DATA_TYPE_TRIGGER, "");
			if (Res != LE_OK)
			{
				LE_INFO("Failed to create Data Hub Output Duplicate Trigger %s", No);
			}
			else
			{
				io_MarkOptional(Dest4);
			}
			io_AddTriggerPushHandler(Dest4, HandleTriggerPush, I);

			char Dest5[50];
			strcpy(Dest5, UC1);
			strcat(Dest5, No);
			strcat(Dest5, "/interface");
			Res = io_CreateOutput(Dest5, IO_DATA_TYPE_NUMERIC, "");
			if (Res != LE_OK)
			{
				LE_INFO("Failed to create Data Hub Output Numeric %s", No);
			}
			else
			{
				io_SetNumericDefault(Dest5, 0);
			}
			io_AddNumericPushHandler(Dest5, HandleInterfacePush, I);
		}
    }
}
