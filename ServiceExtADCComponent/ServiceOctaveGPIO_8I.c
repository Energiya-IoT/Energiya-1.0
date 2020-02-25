#include "legato.h"
#include "interfaces.h"
#include "ServiceOctaveGPIO.h"
#include <linux/i2c-dev.h>

/*
 * ServiceOctaveGPIO_8I.c
 *
 *  	Created on: Oct 31, 2019
 *      Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *      Tel: +48 505 148 375
 */

static uint8_t Address_I2C;

static uint8_t Read(uint8_t Register)
{
	int FD = open("/dev/i2c-4", O_RDWR);
	uint8_t Val = 0;

	if (FD >= 0)
	{
		if (ioctl(FD, I2C_SLAVE_FORCE, Address_I2C) >= 0)
		{
			//ushort Offset = htons(Register);

			if (write(FD, &Register, sizeof(Register)) == sizeof(Register))
			{
				if (read(FD, &Val, 1) == 1)
				{
					LE_INFO("Read OK.");
				}
				else
				{
					LE_INFO("Read Failed.");
				}
			}
		}
		close(FD);
	}
	return Val;
}

static bool Write(uint8_t Register, uint8_t Data)
{
	int FD = open("/dev/i2c-4", O_RDWR);
	bool Res = false;

	if (FD >= 0)
	{
		if (ioctl(FD, I2C_SLAVE_FORCE, Address_I2C) >= 0)
		{
			uint8_t Msg[2];
			Msg[0] = Register;
			Msg[1] = Data;

			if (write(FD, Msg, 2) == 2)
			{
				Res = true;
			}
		}
		close(FD);
	}
	return Res;
}

/*static void OnStateChanege8I(bool State, void* Ctx)
{
	uint8_t ChangedGPIO = Read(0x07);
	while(ChangedGPIO != 0)
	{
		LE_INFO("GPIO Fired!!!");
		uint8_t ValueGPIO = Read(0x09);
		for (uint8_t i = 1; i <= 8; i++)
		{
			if((ChangedGPIO & 0b00000001) == 0b00000001)
			{
				io_PushBoolean(GPIOS[i - 1].Path, IO_NOW, (ValueGPIO & 0b00000001));
				char Str[50];
				sprintf(Str, "Gpio %d - value:%d", i, (ValueGPIO & 0b00000001));
				LE_INFO(Str);
			}
			ChangedGPIO = ChangedGPIO >> 1;
			ValueGPIO = ValueGPIO >> 1;
		}
		ChangedGPIO = Read(0x07);
	}
}*/

static void OnStateChanege8I(bool State, void* Ctx)
{
	uint8_t ChangedGPIO = 1;//Read(0x07);
	while(ChangedGPIO != 0)
	{
		LE_INFO("GPIO Fired!!!");
		uint8_t ValueGPIO = Read(0x09);
		for (uint8_t i = 1; i <= 8; i++)
		{
			uint8_t Val = ValueGPIO & 0b00000001;
			if(GPIOS[i - 1].Value != Val)
			{
				GPIOS[i - 1].Value = Val;
				io_PushBoolean(GPIOS[i - 1].Path, IO_NOW, Val);
				char Str[50];
				sprintf(Str, "Gpio %d - value:%d", i, Val);
				LE_INFO(Str);
			}
			ValueGPIO = ValueGPIO >> 1;
		}
		ChangedGPIO = Read(0x07);
	}
}

static void SetupExpander()
{
	if(Write(0x01, 0XFF))
	{
		LE_INFO("0x01 Setup.");
	}
	else
	{
		LE_INFO("0x01 Failed.");
	}

	if(Write(0x02, 0XFF))
	{
		LE_INFO("0x02 Setup.");
	}
	else
	{
		LE_INFO("0x02 Failed.");
	}

	if(Write(0x05, 0b00100100))
	{
		LE_INFO("0x05 Setup.");
	}
	else
	{
		LE_INFO("0x05 Failed.");
	}

	if(Write(0x06, 0b00000000))
	{
		LE_INFO("0x06 Setup.");
	}
	else
	{
		LE_INFO("0x06 Failed.");
	}
}


static void HandleSettingsAddressPush(double timestamp, double AddressMode, void* contextPtr)
{
	switch((int)AddressMode)
	{
		case 0:
			Address_I2C = 0x20;
			break;
		case 1:
			Address_I2C = 0x21;
			break;
		case 2:
			Address_I2C = 0x22;
			break;
		case 3:
			Address_I2C = 0x23;
			break;
	}
	SetupExpander();
}

void Init_8Inputs()
{
	le_gpioPin8_TryConnectService();
	le_gpioPin8_SetInput(LE_GPIOPIN8_ACTIVE_HIGH);
	Address_I2C = 0x20;

	SetupExpander();

	GPIOS[3].EventRef = le_gpioPin8_AddChangeEventHandler(LE_GPIOPIN8_EDGE_BOTH, OnStateChanege8I, NULL, 0);

	uint8_t StatusGPIO = Read(0x09);

	le_result_t Res = io_CreateOutput("AddOnGPIO/I2C_Address", IO_DATA_TYPE_NUMERIC, "");
	if (Res != LE_OK)
	{
		LE_INFO("Failed to create I2C Address");
	}
	else
	{
		io_SetNumericDefault("AddOnGPIO/I2C_Address", 0);
	}
	AddressHandlerGPIO_I2C = io_AddNumericPushHandler("AddOnGPIO/I2C_Address", HandleSettingsAddressPush, NULL);

	for (uint8_t i = 1; i <= 8; i++)
	{
		char* No1 = "AddOnGPIO";
		char Dest1[50];
		char No[2] = { (char)(48 + i), '\0' };
		strcpy(Dest1, No1);
		strcat(Dest1, "/Input");
		strcat(Dest1, No);
		GPIOS[i - 1].GPIO_NO = i;
		strcpy(GPIOS[i - 1].Path, Dest1);

		le_result_t Res = io_CreateInput(GPIOS[i - 1].Path, IO_DATA_TYPE_BOOLEAN, "");

		if (Res != LE_OK)
		{
			LE_INFO("Failed to create Data Hub Input GPIO");
		}
		else
		{
			io_SetBooleanDefault(GPIOS[i - 1].Path, false);
		}
		io_PushBoolean(GPIOS[i - 1].Path, IO_NOW, ((StatusGPIO & 0b00000001) == 0b00000001));
		GPIOS[i - 1].Value = (StatusGPIO & 0b00000001);

		StatusGPIO = StatusGPIO >> 1;
	}
}
