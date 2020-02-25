#include "legato.h"
#include "interfaces.h"
#include "ServiceOctaveGPIO.h"

/*
 * ServiceOctaveGPIO_II.c
 *
 *  Created on: Oct 3, 2019
 *     Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *         Tel: +48 505 148 375
 */

static void OnStateChanege(bool State, void* Ctx)
{
	struct GPIO* TGPIO = Ctx;
    LE_INFO("GPIO Input State Changed.");
	io_PushBoolean(TGPIO->Path, IO_NOW, State);
}

void Init1_I()
{
    //le_gpioPin42_TryConnectService();
	le_gpioPin42_SetInput(LE_GPIOPIN42_ACTIVE_HIGH);
	GPIOS[0].EventRef = le_gpioPin42_AddChangeEventHandler(LE_GPIOPIN42_EDGE_BOTH, OnStateChanege, &GPIOS[0], 0);
	GPIOS[0].GPIO_NO = 1;
	char* No1 = "AddOnGPIO";
    char Dest1[50];
    strcpy(Dest1, No1);
    strcat(Dest1, "/Input1");
    strcpy(GPIOS[0].Path, Dest1);

    le_result_t Res = io_CreateInput(Dest1, IO_DATA_TYPE_BOOLEAN, "");

	if (Res != LE_OK)
	{
		LE_INFO("Failed to create Data Hub Input GPIO 1");
	}
    else
    {
        io_SetBooleanDefault(Dest1, false);
    }
    io_PushBoolean(Dest1, IO_NOW, le_gpioPin42_Read());

    //le_gpioPin33_TryConnectService();
    le_gpioPin33_SetInput(LE_GPIOPIN33_ACTIVE_HIGH);
    GPIOS[1].EventRef = le_gpioPin33_AddChangeEventHandler(LE_GPIOPIN33_EDGE_BOTH, OnStateChanege, &GPIOS[1], 0);
	GPIOS[1].GPIO_NO = 2;
	char* No2 = "AddOnGPIO";
    char Dest2[50];
    strcpy(Dest2, No2);
    strcat(Dest2, "/Input2");
    strcpy(GPIOS[1].Path, Dest2);

    Res = io_CreateInput(Dest2, IO_DATA_TYPE_BOOLEAN, "");

	if (Res != LE_OK)
	{
		LE_INFO("Failed to create Data Hub Input GPIO 2");
	}
    else
    {
        io_SetBooleanDefault(Dest2, false);
    }
    io_PushBoolean(Dest2, IO_NOW, le_gpioPin33_Read());
}

void Init2_I()
{
    //le_gpioPin13_TryConnectService();
    le_gpioPin13_SetInput(LE_GPIOPIN13_ACTIVE_HIGH);
	GPIOS[2].EventRef = le_gpioPin13_AddChangeEventHandler(LE_GPIOPIN13_EDGE_BOTH, OnStateChanege, &GPIOS[2], 0);
	GPIOS[2].GPIO_NO = 3;
	char* No3 = "AddOnGPIO";
    char Dest3[50];
    strcpy(Dest3, No3);
    strcat(Dest3, "/Input3");
    strcpy(GPIOS[2].Path, Dest3);

    le_result_t Res = io_CreateInput(Dest3, IO_DATA_TYPE_BOOLEAN, "");

	if (Res != LE_OK)
	{
		LE_INFO("Failed to create Data Hub Input GPIO 3");
	}
    else
    {
        io_SetBooleanDefault(Dest3, false);
    }
    io_PushBoolean(Dest3, IO_NOW, le_gpioPin13_Read());

    //le_gpioPin8_TryConnectService();
    le_gpioPin8_SetInput(LE_GPIOPIN8_ACTIVE_HIGH);
	GPIOS[3].EventRef = le_gpioPin8_AddChangeEventHandler(LE_GPIOPIN8_EDGE_BOTH, OnStateChanege, &GPIOS[3], 0);
	GPIOS[3].GPIO_NO = 4;
	char* No4 = "AddOnGPIO";
    char Dest4[50];
    strcpy(Dest4, No4);
    strcat(Dest4, "/Input4");
    strcpy(GPIOS[3].Path, Dest4);

    Res = io_CreateInput(Dest4, IO_DATA_TYPE_BOOLEAN, "");

	if (Res != LE_OK)
	{
		LE_INFO("Failed to create Data Hub Input GPIO 4");
	}
    else
    {
        io_SetBooleanDefault(Dest4, false);
    }
    io_PushBoolean(Dest4, IO_NOW, le_gpioPin8_Read());
}
