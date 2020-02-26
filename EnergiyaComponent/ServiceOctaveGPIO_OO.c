#include "legato.h"

#include "interfaces.h"
#include "ServiceOctaveGPIO.h"

/*
 * ServiceOctaveGPIO_OO.c
 *
 *  Created on: Oct 3, 2019
 *     Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *         Tel: +48 505 148 375
 */

static void HandleGPIOPush(double timestamp, bool enable, void* contextPtr)
{
	struct GPIO* TGPIO = contextPtr;
	LE_INFO("GPIO Output State Changed.");
	switch(TGPIO->GPIO_NO)
	{
		case 1:
			if(enable)
			{
				le_gpioPin42_Activate();
			}
			else
			{
				le_gpioPin42_Deactivate();
			}
			break;

		case 2:
			if(enable)
			{
				le_gpioPin33_Activate();
			}
			else
			{
				le_gpioPin33_Deactivate();
			}
			break;

		case 3:
			if(enable)
			{
				le_gpioPin13_Activate();
			}
			else
			{
				le_gpioPin13_Deactivate();
			}
			break;

		case 4:
			if(enable)
			{
				le_gpioPin8_Activate();
			}
			else
			{
				le_gpioPin8_Deactivate();
			}
			break;
	}
}

void Init1_O()
{
    //le_gpioPin42_TryConnectService();
	le_gpioPin42_EnablePullUp();
	le_gpioPin42_SetPushPullOutput(LE_GPIOPIN42_ACTIVE_HIGH, false);
	GPIOS[0].GPIO_NO = 1;
	char* No1 = "addon_gpio";
    char Dest1[50];
    strcpy(Dest1, No1);
    strcat(Dest1, "/output1");
    strcpy(GPIOS[0].Path, Dest1);
    le_result_t Res = io_CreateOutput(Dest1, IO_DATA_TYPE_BOOLEAN, "");
    if (Res != LE_OK)
    {
    	LE_INFO("Failed to create Output 1");
    }
    io_SetBooleanDefault(Dest1, false);
    GPIOS[0].IoHandlerRef = io_AddBooleanPushHandler(Dest1, HandleGPIOPush, &GPIOS[0]);

    //le_gpioPin33_TryConnectService();
	le_gpioPin33_EnablePullUp();
	le_gpioPin33_SetPushPullOutput(LE_GPIOPIN33_ACTIVE_HIGH, false);
	GPIOS[1].GPIO_NO = 2;
	char* No2 = "addon_gpio";
    char Dest2[50];
    strcpy(Dest2, No2);
    strcat(Dest2, "/output2");
    strcpy(GPIOS[1].Path, Dest2);
    Res = io_CreateOutput(Dest2, IO_DATA_TYPE_BOOLEAN, "");
    if (Res != LE_OK)
    {
        LE_INFO("Failed to create Output 2");
    }
    io_SetBooleanDefault(Dest2, false);
    GPIOS[1].IoHandlerRef = io_AddBooleanPushHandler(Dest2, HandleGPIOPush, &GPIOS[1]);
}

void Init2_O()
{
    //le_gpioPin13_TryConnectService();
	le_gpioPin13_EnablePullUp();
	le_gpioPin13_SetPushPullOutput(LE_GPIOPIN13_ACTIVE_HIGH, false);
	GPIOS[2].GPIO_NO = 3;
	char* No3 = "addon_gpio";
    char Dest3[50];
    strcpy(Dest3, No3);
    strcat(Dest3, "/output3");
    strcpy(GPIOS[2].Path, Dest3);
    le_result_t Res = io_CreateOutput(Dest3, IO_DATA_TYPE_BOOLEAN, "");
    if (Res != LE_OK)
    {
        LE_INFO("Failed to create Output 3");
    }
    io_SetBooleanDefault(Dest3, false);
    GPIOS[2].IoHandlerRef = io_AddBooleanPushHandler(Dest3, HandleGPIOPush, &GPIOS[2]);

    //le_gpioPin8_TryConnectService();
	le_gpioPin8_EnablePullUp();
	le_gpioPin8_SetPushPullOutput(LE_GPIOPIN8_ACTIVE_HIGH, false);
	GPIOS[3].GPIO_NO = 4;
	char* No4 = "addon_gpio";
    char Dest4[50];
    strcpy(Dest4, No4);
    strcat(Dest4, "/output4");
    strcpy(GPIOS[3].Path, Dest4);
    Res = io_CreateOutput(Dest4, IO_DATA_TYPE_BOOLEAN, "");
    if (Res != LE_OK)
    {
        LE_INFO("Failed to create Output 4");
    }
    io_SetBooleanDefault(Dest4, false);
    GPIOS[3].IoHandlerRef = io_AddBooleanPushHandler(Dest4, HandleGPIOPush, &GPIOS[3]);
}
