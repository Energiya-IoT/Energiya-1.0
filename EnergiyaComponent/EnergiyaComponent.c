#include "legato.h"
#include <linux/i2c-dev.h>
#include "stdio.h"
#include "string.h"
#include <netinet/in.h>
#include "interfaces.h"

/*
 * ServiceExtADCComponent.c
 *
 *  Created on: Jul 16, 2018
 *     Company: Energiya
 *	   Project: ServiceExtADC
 *      Author: Bartosz Cieslicki
 *      E-Mail: bartek@energiya.pl
 *         Tel: +48 505 148 375
 *
 *         Ver: 2.0.0
 */

typedef struct
{
	uint8_t Sensor;
	le_ExtAdc_HandlerRefFunc_t HandlerFun;
	void* contextPtr;
	void* Next;
}HRef;

typedef struct
{
	uint8_t Sensor;
	le_ExtAdc_AlertHandlerRefFunc_t HandlerFun;
	le_ExtAdc_Edges_t Edge;
	void* contextPtr;
	void* Next;
}AlertHRef;

struct MinMax
{
	le_ExtAdc_InterfaceType_t Interface;
	double Min;
	double Max;
}MinMaxADC1, MinMaxADC2, MinMaxADC3, MinMaxADC4;

struct EdgesADC
{
	double TooLow;
	double TooHigh;
	double Hysteresis;
}EdgeADC1, EdgeADC2, EdgeADC3, EdgeADC4;

static HRef* Hndlers = NULL ;
static AlertHRef* AlertHndlers = NULL;
static le_mem_PoolRef_t HandlersPool = NULL;
static le_mem_PoolRef_t AlertHandlersPool = NULL;
static char* FDescription;
static uint8_t DeviceAddress = 0;
static uint32_t Timer;
static bool IsLooperRunning;
static le_thread_Ref_t AppThreadRef;
static le_mutex_Ref_t Key;
static le_ExtAdc_ConversionSpeed_t ConversionSpeed;
static le_ExtAdc_Edges_t Saved1, Saved2, Saved3, Saved4;

static bool ReadW(uint8_t Address, uint8_t Register, uint8_t* Data)
{
	bool Res = false;
	const char* NameFD = FDescription;
	if(FDescription != NULL)
	{
		int FD = open(NameFD, O_RDWR);
		if (FD > 0)
		{
			if(ioctl(FD, I2C_SLAVE_FORCE, Address) < 0)
			{
				Res = false;
			}
			else
			{
				if(write(FD, &Register, 1) == 1)
				{
					while((Data[2] & 128) == 128)
					{
						usleep(5000);
						if(read(FD, Data, 3) == 3)
						{
							Res = true;
						}
						else
						{
							Res = false;
						}
					}
				}
				else
				{
					Res = false;
				}
			}
			close(FD);
		}
	}
	return Res;
}

static bool IsHandlerActive()
{
	if((AlertHndlers != NULL) || (Hndlers != NULL))
	{
		return true;
	}
	return false;
}

void le_ExtAdc_MutexLock()
{
	le_mutex_Lock(Key);
	LE_INFO("MUTEX ON");
}

void le_ExtAdc_MutexUnlock()
{
	LE_INFO("MUTEX OFF");
	le_mutex_Unlock(Key);
}

void le_ExtAdc_SetInterface(le_ExtAdc_IOT_ADC_t Sensor, le_ExtAdc_InterfaceType_t Interface)
{
	le_mutex_Lock(Key);
    char Info[50];
	if((Sensor & LE_EXTADC_IOT_ADC1) == LE_EXTADC_IOT_ADC1)
	{
		MinMaxADC1.Interface = Interface;
	    sprintf (Info, "ADC1 - %d", (uint8_t)Interface);
	}

	if((Sensor & LE_EXTADC_IOT_ADC2) == LE_EXTADC_IOT_ADC2)
	{
		MinMaxADC2.Interface = Interface;
	    sprintf (Info, "ADC2 - %d", (uint8_t)Interface);
	}

	if((Sensor & LE_EXTADC_IOT_ADC3) == LE_EXTADC_IOT_ADC3)
	{
		MinMaxADC3.Interface = Interface;
		sprintf (Info, "ADC3 - %d", (uint8_t)Interface);
	}

	if((Sensor & LE_EXTADC_IOT_ADC4) == LE_EXTADC_IOT_ADC4)
	{
		MinMaxADC4.Interface = Interface;
		sprintf (Info, "ADC4 - %d", (uint8_t)Interface);
	}
	LE_INFO(Info);
	le_mutex_Unlock(Key);
}

le_result_t le_ExtAdc_SetDeviceAddress(le_ExtAdc_Pins_t Pin)
{
	le_result_t Res = LE_BAD_PARAMETER;
	le_mutex_Lock(Key);
	switch(Pin)
	{
		case LE_EXTADC_PIN00:
			DeviceAddress = 0b00000000;
			LE_INFO("68h");
			Res = LE_OK;
			break;
		case LE_EXTADC_PIN01:
			DeviceAddress = 0b00001000;
			LE_INFO("6Ch");
			Res = LE_OK;
			break;
		case LE_EXTADC_PIN10:
			DeviceAddress = 0b00000100;
			LE_INFO("6Ah");
			Res = LE_OK;
			break;
		case LE_EXTADC_PIN11:
			DeviceAddress = 0b00001100;
			LE_INFO("6Eh");
			Res = LE_OK;
			break;
	}
	le_mutex_Unlock(Key);
	return Res;
}

le_result_t le_ExtAdc_GetValue(le_ExtAdc_IOT_ADC_t Sensor, uint16_t* RawDataPtr, double* PercentagePtr, double* ConvertedValuePtr)
{

	//1  1	0	1	A2	A1	A0	R/W	RDY	C1	C0	O/C	S1	S0	G1	G0
	le_mutex_Lock(Key);
	uint8_t AdressWrite = 0b11010000 | DeviceAddress;
	uint8_t Data;
	le_ExtAdc_InterfaceType_t Interface;
	double Min;
	double Max;
	switch(Sensor)
	{
		case LE_EXTADC_IOT_ADC1:
			Data = 0b10000000;
			Interface = MinMaxADC1.Interface;
			Min = MinMaxADC1.Min;
			Max = MinMaxADC1.Max;
			break;

		case LE_EXTADC_IOT_ADC2:
			Data = 0b10100000;
			Interface = MinMaxADC2.Interface;
			Min = MinMaxADC2.Min;
			Max = MinMaxADC2.Max;
			break;

		case LE_EXTADC_IOT_ADC3:
			Data = 0b11000000;
			Interface = MinMaxADC3.Interface;
			Min = MinMaxADC3.Min;
			Max = MinMaxADC3.Max;
			break;

		case LE_EXTADC_IOT_ADC4:
			Data = 0b11100000;
			Interface = MinMaxADC4.Interface;
			Min = MinMaxADC4.Min;
			Max = MinMaxADC4.Max;
			break;

		default:
			le_mutex_Unlock(Key);
			return LE_BAD_PARAMETER;
	}

	double MaxResolution = 2000.0;
	double MinResolution = 400.0;

	if(ConversionSpeed == LE_EXTADC_HIGHPRECISION15SPS)
	{
		Data |= 0b00001000;
		MaxResolution = 32000.0;
		MinResolution = 6400.0;
	}

	uint16_t Trials[5] = { 0,0,0,0,0 };
	AdressWrite = AdressWrite >> 1;
	int j;
    for (j = 0; j < 5; j++)
    {
		uint8_t ResArr[3];
		ResArr[2] = 128;
		if(ReadW(AdressWrite, Data, ResArr))
		{
			uint16_t Res = ResArr[0] * 256 + ResArr[1];
			uint16_t Temp[5] = { 0,0,0,0,0 };
			int i;
			for (i = 4; 0 <= i; i--)
			{
				if (Trials[i] < Res)
				{
					Temp[i] = Res;
					break;
				}
				else
				{
					Temp[i] = Trials[i];
				}
			}
			for (; 0 < i; i--)
			{
				Temp[i - 1] = Trials[i];
			}
			for (i = 0; i < 5; i++)
			{
				Trials[i] = Temp[i];
			}
		}
	}

    uint16_t Fin = Trials[2];

	(*RawDataPtr) = Fin;

    if((Interface == LE_EXTADC_MA_020) || (Interface == LE_EXTADC_MA_420))
    {
    	if((Fin < MinResolution) && (Interface == LE_EXTADC_MA_420))
    	{
    		(*PercentagePtr) = 0;
    		le_mutex_Unlock(Key);
    		return LE_UNDERFLOW;
    	}
    	if(Fin > MaxResolution)
    	{
    		(*PercentagePtr) = 100 + (100 * (((double)(Fin - MaxResolution)) / ((double)(MaxResolution - MinResolution))));
    		le_mutex_Unlock(Key);
    		return LE_OVERFLOW;
    	}
    }
    else
    {
    	if(Fin > MaxResolution)
    	{
    		(*PercentagePtr) = 100 + (100 * (((double)(Fin - MaxResolution)) / ((double)(MaxResolution - MinResolution))));
    		le_mutex_Unlock(Key);
    		return LE_OVERFLOW;
    	}
    }

	if(Fin < 0)
	{
		(*PercentagePtr) = 0;
		le_mutex_Unlock(Key);
		return LE_UNDERFLOW;
	}

	double Percent = -1;
	if((Interface == LE_EXTADC_MA_020) || (Interface == LE_EXTADC_MA_420))
    {
		if(Interface == LE_EXTADC_MA_420)
		{
			Percent = (100.00 * (double)(Fin - MinResolution)) / (double)(MaxResolution - MinResolution);
		}
		else
		{
			Percent = (100.00 * (double)Fin) / (double)MaxResolution;
		}
    }
    else
    {
    	Percent = (100.00 * (double)Fin) / (double)MaxResolution;
    }
	(*PercentagePtr) = Percent;
	(*ConvertedValuePtr) = ((Percent / 100.0) * (Max - Min)) + Min;
	le_mutex_Unlock(Key);
	return LE_OK;
}

void* Looper(void* ctxPtr)
{
	//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	while(IsLooperRunning)
	{
		le_mutex_Lock(Key);
		LE_INFO("LOOPER ON");

		bool AlreadyCalculated1 = false;
		bool AlreadyCalculated2 = false;
		bool AlreadyCalculated3 = false;
		bool AlreadyCalculated4 = false;
		uint16_t RawDataPtr1, RawDataPtr2, RawDataPtr3, RawDataPtr4;
		double PercentagePtr1, PercentagePtr2, PercentagePtr3, PercentagePtr4;
		double ConvertedValuePtr1, ConvertedValuePtr2, ConvertedValuePtr3, ConvertedValuePtr4;
		le_ExtAdc_Edges_t TSaved1, TSaved2, TSaved3, TSaved4;

		LE_INFO("Get value trial");
	    HRef* THndlers = Hndlers;

	    while (true)
	    {
			if(THndlers != NULL)
			{
				if((THndlers->Sensor & LE_EXTADC_IOT_ADC1) == LE_EXTADC_IOT_ADC1)
				{
					if((AlreadyCalculated1) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC1, &RawDataPtr1, &PercentagePtr1, &ConvertedValuePtr1) == LE_OK))
					{
						AlreadyCalculated1 = true;
						THndlers->HandlerFun(LE_EXTADC_IOT_ADC1, RawDataPtr1, PercentagePtr1, ConvertedValuePtr1, THndlers->contextPtr);
					}
				}
				if((THndlers->Sensor & LE_EXTADC_IOT_ADC2) == LE_EXTADC_IOT_ADC2)
				{
					if((AlreadyCalculated2) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC2, &RawDataPtr2, &PercentagePtr2, &ConvertedValuePtr2) == LE_OK))
					{
						AlreadyCalculated2 = true;
						THndlers->HandlerFun(LE_EXTADC_IOT_ADC2, RawDataPtr2, PercentagePtr2, ConvertedValuePtr2, THndlers->contextPtr);
					}
				}
				if((THndlers->Sensor & LE_EXTADC_IOT_ADC3) == LE_EXTADC_IOT_ADC3)
				{
					if((AlreadyCalculated3) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC3, &RawDataPtr3, &PercentagePtr3, &ConvertedValuePtr3) == LE_OK))
					{
						AlreadyCalculated3 = true;
						THndlers->HandlerFun(LE_EXTADC_IOT_ADC3, RawDataPtr3, PercentagePtr3, ConvertedValuePtr3, THndlers->contextPtr);
					}
				}
				if((THndlers->Sensor & LE_EXTADC_IOT_ADC4) == LE_EXTADC_IOT_ADC4)
				{
					if((AlreadyCalculated4) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC4, &RawDataPtr4, &PercentagePtr4, &ConvertedValuePtr4) == LE_OK))
					{
						AlreadyCalculated4 = true;
						THndlers->HandlerFun(LE_EXTADC_IOT_ADC4, RawDataPtr4, PercentagePtr4, ConvertedValuePtr4, THndlers->contextPtr);
					}
				}
				THndlers = (HRef*)THndlers->Next;
			}
			else
			{
				break;
			}
	    }

	    AlertHRef* TAHndlers = AlertHndlers;
	    while (true)
	    {
			if(TAHndlers != NULL)
			{
				if((TAHndlers->Sensor & LE_EXTADC_IOT_ADC1) == LE_EXTADC_IOT_ADC1)
				{
					if((AlreadyCalculated1) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC1, &RawDataPtr1, &PercentagePtr1, &ConvertedValuePtr1) == LE_OK))
					{
						AlreadyCalculated1 = true;
						if((ConvertedValuePtr1 > EdgeADC1.TooHigh) && (Saved1 != LE_EXTADC_TOOHIGH))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC1, LE_EXTADC_TOOHIGH, RawDataPtr1, PercentagePtr1, ConvertedValuePtr1, TAHndlers->contextPtr);
							TSaved1 = LE_EXTADC_TOOHIGH;
						}
						else if((ConvertedValuePtr1 < EdgeADC1.TooLow) && (Saved1 != LE_EXTADC_TOOLOW))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC1, LE_EXTADC_TOOLOW, RawDataPtr1, PercentagePtr1, ConvertedValuePtr1, TAHndlers->contextPtr);
							TSaved1 = LE_EXTADC_TOOLOW;
						}
						else
						{
							if(Saved1 == LE_EXTADC_TOOLOW)
							{
								if(ConvertedValuePtr1 >= EdgeADC1.TooLow + EdgeADC1.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC1, LE_EXTADC_NORMAL, RawDataPtr1, PercentagePtr1, ConvertedValuePtr1, TAHndlers->contextPtr);
									TSaved1 = LE_EXTADC_NORMAL;
								}
							}
							else if(Saved1 == LE_EXTADC_TOOHIGH)
							{
								if(ConvertedValuePtr1 <= EdgeADC1.TooHigh - EdgeADC1.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC1, LE_EXTADC_NORMAL, RawDataPtr1, PercentagePtr1, ConvertedValuePtr1, TAHndlers->contextPtr);
									TSaved1 = LE_EXTADC_NORMAL;
								}
							}
						}
					}
				}
				if((TAHndlers->Sensor & LE_EXTADC_IOT_ADC2) == LE_EXTADC_IOT_ADC2)
				{
					if((AlreadyCalculated2) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC2, &RawDataPtr2, &PercentagePtr2, &ConvertedValuePtr2) == LE_OK))
					{
						AlreadyCalculated2 = true;
						if((ConvertedValuePtr2 > EdgeADC2.TooHigh) && (Saved2 != LE_EXTADC_TOOHIGH))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC2, LE_EXTADC_TOOHIGH, RawDataPtr2, PercentagePtr2, ConvertedValuePtr2, TAHndlers->contextPtr);
							TSaved2 = LE_EXTADC_TOOHIGH;
						}
						else if((ConvertedValuePtr2 < EdgeADC2.TooLow) && (Saved2 != LE_EXTADC_TOOLOW))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC2, LE_EXTADC_TOOLOW, RawDataPtr2, PercentagePtr2, ConvertedValuePtr2, TAHndlers->contextPtr);
							TSaved2 = LE_EXTADC_TOOLOW;
						}
						else
						{
							if(Saved2 == LE_EXTADC_TOOLOW)
							{
								if(ConvertedValuePtr2 >= EdgeADC2.TooLow + EdgeADC2.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC2, LE_EXTADC_NORMAL, RawDataPtr2, PercentagePtr2, ConvertedValuePtr2, TAHndlers->contextPtr);
									TSaved2 = LE_EXTADC_NORMAL;
								}
							}
							else if(Saved2 == LE_EXTADC_TOOHIGH)
							{
								if(ConvertedValuePtr2 <= EdgeADC2.TooHigh - EdgeADC2.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC2, LE_EXTADC_NORMAL, RawDataPtr2, PercentagePtr2, ConvertedValuePtr2, TAHndlers->contextPtr);
									TSaved2 = LE_EXTADC_NORMAL;
								}
							}
						}
					}
				}
				if((TAHndlers->Sensor & LE_EXTADC_IOT_ADC3) == LE_EXTADC_IOT_ADC3)
				{
					if((AlreadyCalculated3) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC3, &RawDataPtr3, &PercentagePtr3, &ConvertedValuePtr3) == LE_OK))
					{
						AlreadyCalculated3 = true;
						if((ConvertedValuePtr3 > EdgeADC3.TooHigh) && (Saved3 != LE_EXTADC_TOOHIGH))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC3, LE_EXTADC_TOOHIGH, RawDataPtr3, PercentagePtr3, ConvertedValuePtr3, TAHndlers->contextPtr);
							TSaved3 = LE_EXTADC_TOOHIGH;
						}
						else if((ConvertedValuePtr3 < EdgeADC3.TooLow) && (Saved3 != LE_EXTADC_TOOLOW))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC3, LE_EXTADC_TOOLOW, RawDataPtr3, PercentagePtr3, ConvertedValuePtr3, TAHndlers->contextPtr);
							TSaved3 = LE_EXTADC_TOOLOW;
						}
						else
						{
							if(Saved3 == LE_EXTADC_TOOLOW)
							{
								if(ConvertedValuePtr3 >= EdgeADC3.TooLow + EdgeADC3.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC3, LE_EXTADC_NORMAL, RawDataPtr3, PercentagePtr3, ConvertedValuePtr3, TAHndlers->contextPtr);
									TSaved3 = LE_EXTADC_NORMAL;
								}
							}
							else if(Saved3 == LE_EXTADC_TOOHIGH)
							{
								if(ConvertedValuePtr3 <= EdgeADC3.TooHigh - EdgeADC3.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC3, LE_EXTADC_NORMAL, RawDataPtr3, PercentagePtr3, ConvertedValuePtr3, TAHndlers->contextPtr);
									TSaved3 = LE_EXTADC_NORMAL;
								}
							}
						}
					}
				}
				if((TAHndlers->Sensor & LE_EXTADC_IOT_ADC4) == LE_EXTADC_IOT_ADC4)
				{
					if((AlreadyCalculated4) || (le_ExtAdc_GetValue(LE_EXTADC_IOT_ADC4, &RawDataPtr4, &PercentagePtr4, &ConvertedValuePtr4) == LE_OK))
					{
						AlreadyCalculated4 = true;
						if((ConvertedValuePtr4 > EdgeADC4.TooHigh) && (Saved4 != LE_EXTADC_TOOHIGH))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC4, LE_EXTADC_TOOHIGH, RawDataPtr4, PercentagePtr4, ConvertedValuePtr4, TAHndlers->contextPtr);
							TSaved4 = LE_EXTADC_TOOHIGH;
						}
						else if((ConvertedValuePtr4 < EdgeADC4.TooLow) && (Saved4 != LE_EXTADC_TOOLOW))
						{
							TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC4, LE_EXTADC_TOOLOW, RawDataPtr4, PercentagePtr4, ConvertedValuePtr4, TAHndlers->contextPtr);
							TSaved4 = LE_EXTADC_TOOLOW;
						}
						else
						{
							if(Saved4 == LE_EXTADC_TOOLOW)
							{
								if(ConvertedValuePtr4 >= EdgeADC4.TooLow + EdgeADC4.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC4, LE_EXTADC_NORMAL, RawDataPtr4, PercentagePtr4, ConvertedValuePtr4, TAHndlers->contextPtr);
									TSaved4 = LE_EXTADC_NORMAL;
								}
							}
							else if(Saved4 == LE_EXTADC_TOOHIGH)
							{
								if(ConvertedValuePtr4 <= EdgeADC4.TooHigh - EdgeADC4.Hysteresis)
								{
									TAHndlers->HandlerFun(LE_EXTADC_IOT_ADC4, LE_EXTADC_NORMAL, RawDataPtr4, PercentagePtr4, ConvertedValuePtr4, TAHndlers->contextPtr);
									TSaved4 = LE_EXTADC_NORMAL;
								}
							}
						}
					}
				}
				TAHndlers = (AlertHRef*)TAHndlers->Next;
			}
			else
			{
				break;
			}
		}

		Saved1 = TSaved1;
		Saved2 = TSaved2;
		Saved3 = TSaved3;
		Saved4 = TSaved4;

		le_mutex_Unlock(Key);
		LE_INFO("LOOPER OFF");
		//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		//usleep(Timer);
		if(IsLooperRunning)
		{
			uint32_t SmallTimer = Timer % 1000;
			usleep(SmallTimer * 1000);
			uint32_t BigTimer = Timer / 1000;
			uint32_t i = 0;
			if(IsLooperRunning)
			{
				for(i = 0; i < BigTimer; i++)
				{
					if(IsLooperRunning)
					{
						sleep(1);
					}
				}
			}
		}
		//pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	}
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	return NULL;
}

le_ExtAdc_OnValueHandlerRef_t le_ExtAdc_AddOnValueHandler(uint8_t Sens, le_ExtAdc_HandlerRefFunc_t handlerPtr, void* contextPtr)
{
	le_mutex_Lock(Key);

	HRef* OnValueHandler = Hndlers;
	if (Hndlers == NULL)
	{
		OnValueHandler = (HRef*)le_mem_ForceAlloc(HandlersPool);
		Hndlers = OnValueHandler;
	}
	else
	{
		while (true)
		{
			if (OnValueHandler->Next == NULL)
			{
				OnValueHandler->Next = (HRef*)le_mem_ForceAlloc(HandlersPool);
				OnValueHandler = OnValueHandler->Next;
				break;
			}
			else
			{
				OnValueHandler = OnValueHandler->Next;
			}
		}
	}

	OnValueHandler->Next = NULL;
	OnValueHandler->Sensor = Sens;
	OnValueHandler->HandlerFun = handlerPtr;
	OnValueHandler->contextPtr = contextPtr;
	if(!IsLooperRunning)
	{
		IsLooperRunning = true;
		AppThreadRef = le_thread_Create("AppThread", Looper, NULL);
		le_thread_SetJoinable(AppThreadRef);
		le_thread_Start(AppThreadRef);
	}
	le_mutex_Unlock(Key);
	return (le_ExtAdc_OnValueHandlerRef_t)OnValueHandler;
}

void le_ExtAdc_RemoveOnValueHandler(le_ExtAdc_OnValueHandlerRef_t HandlerRef)
{
	le_mutex_Lock(Key);
	HRef* OnReceiveHandler = Hndlers;
	HRef* PreviousHandler = Hndlers;
	while (true)
	{
		if (OnReceiveHandler != NULL)
		{
			if (OnReceiveHandler == (HRef*)HandlerRef)
			{
				if (OnReceiveHandler == Hndlers)
				{
					Hndlers = OnReceiveHandler->Next;
				}
				else
				{
					PreviousHandler->Next = OnReceiveHandler->Next;
				}
				le_mem_Release(OnReceiveHandler);
				if(!IsHandlerActive())
				{
					IsLooperRunning = false;
					if(AppThreadRef != NULL)
					{
						//le_thread_Cancel(AppThreadRef);
						le_thread_Join(AppThreadRef, NULL);
						AppThreadRef = NULL;
					}
				}
				break;
			}
			else
			{
				PreviousHandler = OnReceiveHandler;
				OnReceiveHandler = OnReceiveHandler->Next;
			}
		}
		else
		{
			break;
		}
	}
	le_mutex_Unlock(Key);
}

le_ExtAdc_OnAlertHandlerRef_t le_ExtAdc_AddOnAlertHandler(uint8_t Sens, le_ExtAdc_AlertHandlerRefFunc_t handlerPtr, void* contextPtr)
{
	le_mutex_Lock(Key);

	AlertHRef* OnAlertHandler = AlertHndlers;
	if (AlertHndlers == NULL)
	{
		OnAlertHandler = (AlertHRef*)le_mem_ForceAlloc(AlertHandlersPool);
		AlertHndlers = OnAlertHandler;
	}
	else
	{
		while (true)
		{
			if (OnAlertHandler->Next == NULL)
			{
				OnAlertHandler->Next = (AlertHRef*)le_mem_ForceAlloc(AlertHandlersPool);
				OnAlertHandler = OnAlertHandler->Next;
				break;
			}
			else
			{
				OnAlertHandler = OnAlertHandler->Next;
			}
		}
	}

	OnAlertHandler->Next = NULL;
	OnAlertHandler->Sensor = Sens;
	OnAlertHandler->HandlerFun = handlerPtr;
	OnAlertHandler->contextPtr = contextPtr;
	if(!IsLooperRunning)
	{
		IsLooperRunning = true;
		AppThreadRef = le_thread_Create("AppThread", Looper, NULL);
		le_thread_SetJoinable(AppThreadRef);
		le_thread_Start(AppThreadRef);
	}
	le_mutex_Unlock(Key);
	return (le_ExtAdc_OnAlertHandlerRef_t)OnAlertHandler;
}

void le_ExtAdc_RemoveOnAlertHandler(le_ExtAdc_OnAlertHandlerRef_t HandlerRef)
{
	le_mutex_Lock(Key);
	AlertHRef* OnReceiveHandler = AlertHndlers;
	AlertHRef* PreviousHandler = AlertHndlers;
	while (true)
	{
		if (OnReceiveHandler != NULL)
		{
			if (OnReceiveHandler == (AlertHRef*)HandlerRef)
			{
				if (OnReceiveHandler == AlertHndlers)
				{
					AlertHndlers = OnReceiveHandler->Next;
				}
				else
				{
					PreviousHandler->Next = OnReceiveHandler->Next;
				}
				le_mem_Release(OnReceiveHandler);
				if(!IsHandlerActive())
				{
					IsLooperRunning = false;
					if(AppThreadRef != NULL)
					{
						//le_thread_Cancel(AppThreadRef);
						le_thread_Join(AppThreadRef, NULL);
						AppThreadRef = NULL;
					}
				}
				break;
			}
			else
			{
				PreviousHandler = OnReceiveHandler;
				OnReceiveHandler = OnReceiveHandler->Next;
			}
		}
		else
		{
			break;
		}
	}
	le_mutex_Unlock(Key);
}

le_result_t le_ExtAdc_SetConverter(le_ExtAdc_IOT_ADC_t Sensor, double Min, double Max)
{
	le_mutex_Lock(Key);
	if(Min < Max)
	{
		if((Sensor & LE_EXTADC_IOT_ADC1) == LE_EXTADC_IOT_ADC1)
		{
			MinMaxADC1.Min = Min;
			MinMaxADC1.Max = Max;
		}

		if((Sensor & LE_EXTADC_IOT_ADC2) == LE_EXTADC_IOT_ADC2)
		{
			MinMaxADC2.Min = Min;
			MinMaxADC2.Max = Max;
		}

		if((Sensor & LE_EXTADC_IOT_ADC3) == LE_EXTADC_IOT_ADC3)
		{
			MinMaxADC3.Min = Min;
			MinMaxADC3.Max = Max;
		}

		if((Sensor & LE_EXTADC_IOT_ADC4) == LE_EXTADC_IOT_ADC4)
		{
			MinMaxADC4.Min = Min;
			MinMaxADC4.Max = Max;
		}
		le_mutex_Unlock(Key);
		return LE_OK;
	}
	else
	{
		le_mutex_Unlock(Key);
		return LE_BAD_PARAMETER;
	}
}

le_result_t le_ExtAdc_SetFileDescriptor(const char* Name)
{
	if((sizeof(Name) > 200) || (sizeof(Name) < 4))
	{
		FDescription = NULL;
		return LE_BAD_PARAMETER;
	}
	else
	{
		le_mutex_Lock(Key);
		FDescription = strdup(Name);;
		le_mutex_Unlock(Key);
		return LE_OK;
	}
}

le_result_t le_ExtAdc_SetEdges(le_ExtAdc_IOT_ADC_t Sensor, double TooLow, double TooHigh, double Hysteresis)
{
	le_mutex_Lock(Key);
	if((TooLow < TooHigh) && ((TooHigh - TooLow) / 2.0) > Hysteresis)
	{
		if((Sensor & LE_EXTADC_IOT_ADC1) == LE_EXTADC_IOT_ADC1)
		{
			EdgeADC1.TooLow = TooLow;
			EdgeADC1.TooHigh = TooHigh;
			EdgeADC1.Hysteresis = Hysteresis;
		}

		if((Sensor & LE_EXTADC_IOT_ADC2) == LE_EXTADC_IOT_ADC2)
		{
			EdgeADC2.TooLow = TooLow;
			EdgeADC2.TooHigh = TooHigh;
			EdgeADC2.Hysteresis = Hysteresis;
		}

		if((Sensor & LE_EXTADC_IOT_ADC3) == LE_EXTADC_IOT_ADC3)
		{
			EdgeADC3.TooLow = TooLow;
			EdgeADC3.TooHigh = TooHigh;
			EdgeADC3.Hysteresis = Hysteresis;
		}

		if((Sensor & LE_EXTADC_IOT_ADC4) == LE_EXTADC_IOT_ADC4)
		{
			EdgeADC4.TooLow = TooLow;
			EdgeADC4.TooHigh = TooHigh;
			EdgeADC4.Hysteresis = Hysteresis;
		}
		le_mutex_Unlock(Key);
		return LE_OK;
	}
	else
	{
		le_mutex_Unlock(Key);
		return LE_BAD_PARAMETER;
	}
}

le_result_t le_ExtAdc_SetInterval(uint32_t Interval)
{
	Timer = Interval;
	return LE_OK;
}

le_result_t le_ExtAdc_SetConversionSpeed(le_ExtAdc_ConversionSpeed_t Speed)
{
	ConversionSpeed = Speed;
	return LE_OK;
}

void Init(uint8_t IoTType);
void DeleteAddOnResources();
void DeleteMainResources();

// The function is for mangOH yellow
/*void Write(uint8_t Address, uint8_t Register, uint8_t Data)
{
	int FD = open("/dev/i2c-8", O_RDWR);
	if (FD > 0)
	{
		if(ioctl(FD, I2C_SLAVE_FORCE, Address) < 0)
		{
			LE_INFO("Error");
		}
		else
		{
			uint8_t Msg[2];
			Msg[0] = Register;
			Msg[1] = Data;

			if(write(FD, Msg ,2) == 2)
			{
				LE_INFO("OK");
			}
			else
			{
				LE_INFO("Error");
			}
		}
	}

	if(FD > 0)
	{
		close(FD);
	}
}*/

int ReadEEPROM()
{
  int FD = open("/dev/i2c-4", O_RDWR);
  char BuffVendor[32] = {0};
  char BuffProduct[32] = {0};
  uint16_t OffSetVendor = 4;
  uint16_t OffSetProduct = 36;
  uint8_t Initiated = 0;

  if (FD >= 0)
  {
	  if (ioctl(FD, I2C_SLAVE_FORCE, 0x52) >= 0)
	  {
		  ushort Offset = htons(OffSetVendor);
		  if (write(FD, &Offset, sizeof(Offset)) == sizeof(Offset))
		  {
			  if (read(FD, BuffVendor, 32) == 32)
			  {
				  LE_INFO(BuffVendor);
			  }
		  }

		  Offset = htons(OffSetProduct);
		  if (write(FD, &Offset, sizeof(Offset)) == sizeof(Offset))
		  {
			  if (read(FD, BuffProduct, 32) == 32)
			  {
				  LE_INFO(BuffProduct);
			  }
		  }

		  const char* Vendor = "ENERGIYA made in Poland";
		  const char* Product = "Universal Converter 4i";
		  if(strncmp(BuffVendor, Vendor, strlen(Vendor)) == 0)
		  {
			  LE_INFO("Init Octave...");
			  Initiated++;
		  }
		  if(strncmp(BuffProduct, Product, strlen(Product)) == 0)
		  {
			  Initiated++;
		  }
	  }
	  close(FD);
  }
  switch(Initiated)
  {
  	  case 1:
		le_gpioPin8_TryConnectService();
	        le_gpioPin8_EnablePullUp();
	        le_gpioPin8_SetPushPullOutput(LE_GPIOPIN8_ACTIVE_HIGH, false);
  		DeleteMainResources(Initiated);
  		Init(Initiated);
  		break;

  	  case 2:
		le_gpioPin8_TryConnectService();
	        le_gpioPin8_EnablePullUp();
	        le_gpioPin8_SetPushPullOutput(LE_GPIOPIN8_ACTIVE_HIGH, false);
  		Init(Initiated);
  		break;

  	  default:
		  //Remove resources
		  DeleteAddOnResources();
		  DeleteMainResources();
		  le_appCtrl_ConnectService();
		  le_appCtrl_Stop("Energiya");
		  le_appCtrl_DisconnectService();
		 
		  break;
  }
  return 0;
}

COMPONENT_INIT
{
	sleep(10);
	IsLooperRunning = false;
	Saved1 = LE_EXTADC_NORMAL;
	Saved2 = LE_EXTADC_NORMAL;
	Saved3 = LE_EXTADC_NORMAL;
	Saved4 = LE_EXTADC_NORMAL;
	//all four inputs are set up for 4-20mA
	MinMaxADC1.Interface = LE_EXTADC_MA_420;
	MinMaxADC1.Min = 4;
	MinMaxADC1.Max = 20;
	MinMaxADC2.Interface = LE_EXTADC_MA_420;
	MinMaxADC2.Min = 4;
	MinMaxADC2.Max = 20;
	MinMaxADC3.Interface = LE_EXTADC_MA_420;
	MinMaxADC3.Min = 4;
	MinMaxADC3.Max = 20;
	MinMaxADC4.Interface = LE_EXTADC_MA_420;
	MinMaxADC4.Min = 4;
	MinMaxADC4.Max = 20;
	EdgeADC1.TooLow = 0;
	EdgeADC1.TooHigh = 1000;
	EdgeADC1.Hysteresis = 10;
	EdgeADC2.TooLow = 0;
	EdgeADC2.TooHigh = 1000;
	EdgeADC2.Hysteresis = 10;
	EdgeADC3.TooLow = 0;
	EdgeADC3.TooHigh = 1000;
	EdgeADC3.Hysteresis = 10;
	EdgeADC4.TooLow = 0;
	EdgeADC4.TooHigh = 1000;
	EdgeADC4.Hysteresis = 10;
	ConversionSpeed = LE_EXTADC_HIGHPRECISION15SPS;

	Key = le_mutex_CreateRecursive("Key");
	HandlersPool = le_mem_CreatePool("Hnd", sizeof(HRef));
	AlertHandlersPool = le_mem_CreatePool("AlertHnd", sizeof(AlertHRef));
	Timer = 10000;
	le_ExtAdc_SetFileDescriptor("/dev/i2c-4");

	//first way has to be down, second up on Universal Converter enum LE_EXTADC_PIN01.
	//both ways cannot be down, because address 68h is reserved on mangOH red
	le_ExtAdc_SetDeviceAddress(LE_EXTADC_PIN01);

    //CARD_DETECT EEPROM MangOH red GPIO33
	//le_gpioPin25_TryConnectService();
	le_gpioPin25_ConnectService();
	le_gpioPin25_EnablePullUp();
	le_gpioPin25_SetPushPullOutput(LE_GPIOPIN25_ACTIVE_HIGH, true);

	//CARD_DETECT EEPROM MangOH mangOH yellow
	//Write(0x3E, 0x0E, 0);
	//Write(0x3E, 0x10, 0b00000001);

	//Read EEPROM and create/remove resources
	ReadEEPROM();

	le_gpioPin25_Deactivate();
	le_gpioPin25_DisconnectService();

	//Write(0x3E,0x10, 0b00000000);
	sleep(10);
}

