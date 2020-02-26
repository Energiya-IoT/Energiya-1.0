

/*
 * ====================== WARNING ======================
 *
 * THE CONTENTS OF THIS FILE HAVE BEEN AUTO-GENERATED.
 * DO NOT MODIFY IN ANY WAY.
 *
 * ====================== WARNING ======================
 */


#ifndef LE_EXTADC_INTERFACE_H_INCLUDE_GUARD
#define LE_EXTADC_INTERFACE_H_INCLUDE_GUARD


#include "legato.h"


//--------------------------------------------------------------------------------------------------
/**
 * Get the server service reference
 */
//--------------------------------------------------------------------------------------------------
le_msg_ServiceRef_t le_ExtAdc_GetServiceRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the client session reference for the current message
 */
//--------------------------------------------------------------------------------------------------
le_msg_SessionRef_t le_ExtAdc_GetClientSessionRef
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the server and advertise the service.
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_AdvertiseService
(
    void
);


//--------------------------------------------------------------------------------------------------
/**
 * External ADCs on IoT 
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_EXTADC_IOT_ADC1 = 0x1,        ///< sensor no. 1 
    LE_EXTADC_IOT_ADC2 = 0x2,        ///< sensor no. 2 
    LE_EXTADC_IOT_ADC3 = 0x4,        ///< sensor no. 3 
    LE_EXTADC_IOT_ADC4 = 0x8        ///< sensor no. 4 
}
le_ExtAdc_IOT_ADC_t;


//--------------------------------------------------------------------------------------------------
/**
 * ENUM Edges
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_EXTADC_NORMAL = 0,
        ///< When value back to normal
    LE_EXTADC_TOOLOW = 1,
        ///< When value is too low
    LE_EXTADC_TOOHIGH = 2
        ///< When value is too high
}
le_ExtAdc_Edges_t;


//--------------------------------------------------------------------------------------------------
/**
 * ENUM corresponds with hardware switches on IoT. 
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_EXTADC_PIN00 = 0,
        ///< when both ways are down - address 0x68 is set up.
    LE_EXTADC_PIN01 = 1,
        ///< when first is down, second is up - address 0x6C is set up.
    LE_EXTADC_PIN10 = 2,
        ///< when first is up, second is down - address 0x6A is set up.
    LE_EXTADC_PIN11 = 3
        ///< when both ways are up - address 0x6E is set up.
}
le_ExtAdc_Pins_t;


//--------------------------------------------------------------------------------------------------
/**
 * ENUM corresponds with hardware pins on IoT. 
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_EXTADC_MA_020 = 0,
        ///< Interface 0-20mA
    LE_EXTADC_MA_420 = 1,
        ///< Interface 4-20mA
    LE_EXTADC_V = 2
        ///< Interface 0-10V
}
le_ExtAdc_InterfaceType_t;


//--------------------------------------------------------------------------------------------------
/**
 * ENUM conversion speeds. 
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LE_EXTADC_LOWPRECISION240SPS = 0,
        ///< Quick conversion with low precision 240 SPS (12 bits)
    LE_EXTADC_HIGHPRECISION15SPS = 1
        ///< Slow conversion with high precision 15 SPS (16 bits)
}
le_ExtAdc_ConversionSpeed_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_ExtAdc_OnValue'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_ExtAdc_OnValueHandler* le_ExtAdc_OnValueHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference type used by Add/Remove functions for EVENT 'le_ExtAdc_OnAlert'
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_ExtAdc_OnAlertHandler* le_ExtAdc_OnAlertHandlerRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_ExtAdc_AddOnValueHandler'
 * 
 * This event provides reading from specific sensor.
 * Reading will be returned every x number of milliseconds (Default 10000) 
 * Default value can be changed by SetInterval(uint32 Interval)
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_ExtAdc_HandlerRefFunc_t)
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< Sensor
    uint16_t RawData,
        ///< Raw data 15bit
    double Percentage,
        ///< Reading in percent
    double ConvertedValue,
        ///< Returns converted value - see SetConverter(IOT_ADC Sensor, double Min, double Max)
    void* contextPtr
        ///<
);

//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_ExtAdc_AddOnAlertHandler'
 * 
 * This event fires when value is too low, too high or back to normal
 * Edges are set by function SetEdges(IOT_ADC Sensor, double TooLow, double TooHigh, double Hysteresis)
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_ExtAdc_AlertHandlerRefFunc_t)
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< Sensor
    le_ExtAdc_Edges_t Edge,
        ///< Notify if value is too low, too high or back to normal - see SetEdges(IOT_ADC Sensor, double TooLow, double TooHigh, double Hysteresis)
    uint16_t RawData,
        ///< Raw data 15bit
    double Percentage,
        ///< Reading in percent
    double ConvertedValue,
        ///< Returns converted value - see SetConverter(IOT_ADC Sensor, double Min, double Max)
    void* contextPtr
        ///<
);


//--------------------------------------------------------------------------------------------------
/**
 ** Set device address. Device address has to be the same in the program and on IoT.
 ** Hardware switch allows to setup address on IoT. Hardware combination must be passed to the program through the function.
 * 
 * Returns:
 * LE_OK if Successful.
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetDeviceAddress
(
    le_ExtAdc_Pins_t Pin
        ///< [IN] enum le_ExtAdc_Pins_t
);



//--------------------------------------------------------------------------------------------------
/**
 * Set conversion speed.
 * 
 * Returns:
 * LE_OK if Successful.
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetConversionSpeed
(
    le_ExtAdc_ConversionSpeed_t Speed
        ///< [IN] enum le_ExtAdc_ConvertionSpeed_t
);



//--------------------------------------------------------------------------------------------------
/**
 * Set file descriptor. 
 * 
 * Returns:
 * LE_OK if Successful.
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetFileDescriptor
(
    const char* LE_NONNULL Name
        ///< [IN] File descriptor. Example "/dev/i2c-0"
);



//--------------------------------------------------------------------------------------------------
/**
 * Get the values of an ADC input from specific sensor.
 * 
 * Returns:
 * LE_OK if Successful.
 * LE_OVERFLOW if raw value is too high
 * LE_UNDERFLOW if raw value is too low
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_GetValue
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< [IN] Sensor to read
    uint16_t* RawDataPtr,
        ///< [OUT] Raw data 15bit
    double* PercentagePtr,
        ///< [OUT] Reading in percent
    double* ConvertedValuePtr
        ///< [OUT] Returns converted value - see SetConverter(double Min, double Max)
);



//--------------------------------------------------------------------------------------------------
/**
 * Function setup interval (in millisecond). 
 * 
 * Set the time interval which elapses between two reading/checking, provided by events.
 * 
 * Returns:
 * LE_OK if Successful.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetInterval
(
    uint32_t Interval
        ///< [IN] In milliseconds
);



//--------------------------------------------------------------------------------------------------
/**
 * Function setup Min and Max achievable by connected unit. 
 * 
 * Convertible value will be calculated based on the real device Min and Max
 * Returns:
 * LE_OK if Successful.
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetConverter
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< [IN]
    double Min,
        ///< [IN] Device Min value
    double Max
        ///< [IN] Device Max value
);



//--------------------------------------------------------------------------------------------------
/**
 * Function setup interface on specific sensor. 
 * 
 * Available interfaces: 4-20mA and 0-10V
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_SetInterface
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< [IN]
    le_ExtAdc_InterfaceType_t Interface
        ///< [IN] Sensor Interface
);



//--------------------------------------------------------------------------------------------------
/**
 * Function setup edges and Hysteresis. Event OnAlert will be fired, based on these parameters.
 *  
 * Returns:
 * LE_OK if Successful.
 * LE_BAD_PARAMETER if bad parameter
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_ExtAdc_SetEdges
(
    le_ExtAdc_IOT_ADC_t Sensor,
        ///< [IN] Sensor
    double TooLow,
        ///< [IN] Cannot be higher by TooLow parameter.
    double TooHigh,
        ///< [IN] Cannot be lower by TooLow parameter.
    double Hysteresis
        ///< [IN] Hysteresis
);



//--------------------------------------------------------------------------------------------------
/**
 * Function locks I2C communication in the component. 
 * Should be used to avoid conflict with other I2C call
 * 
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_MutexLock
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Function unlocks I2C communication in the component. 
 * 
 * 
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_MutexUnlock
(
    void
);



//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_ExtAdc_OnValue'
 */
//--------------------------------------------------------------------------------------------------
le_ExtAdc_OnValueHandlerRef_t le_ExtAdc_AddOnValueHandler
(
    uint8_t Sensor,
        ///< [IN] Sensor to read can be combined. For example LE_EXTADC_IOT_ADC1|LE_EXTADC_IOT_ADC2
    le_ExtAdc_HandlerRefFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_ExtAdc_OnValue'
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_RemoveOnValueHandler
(
    le_ExtAdc_OnValueHandlerRef_t handlerRef
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Add handler function for EVENT 'le_ExtAdc_OnAlert'
 */
//--------------------------------------------------------------------------------------------------
le_ExtAdc_OnAlertHandlerRef_t le_ExtAdc_AddOnAlertHandler
(
    uint8_t Sensor,
        ///< [IN] Sensor to read can be combined. For example LE_EXTADC_IOT_ADC1|LE_EXTADC_IOT_ADC2
    le_ExtAdc_AlertHandlerRefFunc_t handlerPtr,
        ///< [IN]
    void* contextPtr
        ///< [IN]
);



//--------------------------------------------------------------------------------------------------
/**
 * Remove handler function for EVENT 'le_ExtAdc_OnAlert'
 */
//--------------------------------------------------------------------------------------------------
void le_ExtAdc_RemoveOnAlertHandler
(
    le_ExtAdc_OnAlertHandlerRef_t handlerRef
        ///< [IN]
);


#endif // LE_EXTADC_INTERFACE_H_INCLUDE_GUARD