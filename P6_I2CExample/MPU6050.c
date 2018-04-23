#include "msp430g2553.h"
#include "MPU6050.h"
#include "math.h" // For atan();
#include <stdint.h>
/*-----------------------------------------------------------------------------*/
/* Local Constant definitions */
/*-----------------------------------------------------------------------------*/
/* AFS_SEL | Full Scale Range | LSB Sensitivity
* --------+------------------+----------------
* 0       | +/- 2g           | 16384 LSB/mg
* 1       | +/- 4g           | 8192 LSB/mg
* 2       | +/- 8g           | 4096 LSB/mg
* 3       | +/- 16g          | 2043 LSB/mg
*/
#define SCALED_ACC_2G       16384
#define SCALED_ACC_4G       8192
#define SCALED_ACC_8G       4096
#define SCALED_ACC_16G      2043

/* FS_SEL | Full Scale Range   | LSB Sensitivity
* -------+--------------------+----------------
* 0      | +/- 250 degrees/s  | 131 LSB/deg/s
* 1      | +/- 500 degrees/s  | 65.5 LSB/deg/s
* 2      | +/- 1000 degrees/s | 32.8 LSB/deg/s
* 3      | +/- 2000 degrees/s | 16.4 LSB/deg/s
*/
#define SCALED_GYRO_250     131.0
#define SCALED_GYRO_500     65.5
#define SCALED_GYRO_1000    32.8
#define SCALED_GYRO_2000    16.4

/*
*   Note:
*          |   ACCELEROMETER    |           GYROSCOPE
* DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
* ---------+-----------+--------+-----------+--------+-------------
* 0        | 260Hz     | 0ms    | 256Hz     | 0.98ms | 8kHz
* 1        | 184Hz     | 2.0ms  | 188Hz     | 1.9ms  | 1kHz
* 2        | 94Hz      | 3.0ms  | 98Hz      | 2.8ms  | 1kHz
* 3        | 44Hz      | 4.9ms  | 42Hz      | 4.8ms  | 1kHz
* 4        | 21Hz      | 8.5ms  | 20Hz      | 8.3ms  | 1kHz
* 5        | 10Hz      | 13.8ms | 10Hz      | 13.4ms | 1kHz
* 6        | 5Hz       | 19.0ms | 5Hz       | 18.6ms | 1kHz
* 7        |   -- Reserved --   |   -- Reserved --   | Reserved
*/


/*-----------------------------------------------------------------------------*/
/* Local Macro definitions */
/*-----------------------------------------------------------------------------*/
// Note: This Macro use for Bigendian but MSP430 is little endien => revert
#define CONVERT_TO_16BIT(MSB, LSB)      (((uint16_t)(MSB) << 8) | (uint16_t)(LSB))
#define ABS(x) (x < 0 ? -x : x)
/*-----------------------------------------------------------------------------*/
/* Local Data type definitions */
/*-----------------------------------------------------------------------------*/
#define dt 0.015f
/*-----------------------------------------------------------------------------*/
/* Global variables */
/*-----------------------------------------------------------------------------*/

/* Offset value to calibrate Gyro */
static int16_t Gyro_OffsetValueX = 0;
static int16_t Gyro_OffsetValueY = 0;
static int16_t Gyro_OffsetValueZ = 0;

/* Off set value to calibrate Acc*/
static int16_t Acc_OffsetValueX = 0;
static int16_t Acc_OffsetValueY = 0;
static int16_t Acc_OffsetValueZ = 0;

/* Scale Value config for ACC - default is 2G*/
static float Acc_scaleValue  = SCALED_ACC_2G;

/* Scale Value config for GYRO - default is 2G*/
static float Gyro_scaleValue = SCALED_GYRO_250;

/*-----------------------------------------------------------------------------*/
/* Function prototypes */
/*-----------------------------------------------------------------------------*/

void MPU6050_Init(unsigned char ACC_SCALE_CONFIG, unsigned char GYRO_SCALE_CONFIG);

unsigned char MPU6050_CheckI2C(void);

/* MPU6050 test configure of register*/
unsigned char MPU6050_TestRegConfig(void);

void MPU6050_Calibrate_Gyro(void);
/* Raw Acc Value*/
void MPU6050_GetAccValueRaw(void * pValue);

/* Value in degree/s */
void MPU6050_AccConvertData(ACC_DATA_RAW rawValue, void * scaledData);

/* Raw Gyro Value to m/s^2*/
void MPU6050_GetGyroValueRaw(void * pValue);

/* Convert to m/s^2*/
void PMU6050_GyroConvertData(GYRO_DATA_RAW rawValue, void * scaledData);

/*-----------------------------------------------------------------------------*/
/* Function implementations                                                    */
/*-----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------
Function    : MPU6050_Init
Purpose     : Init and configure for MPU6050
Parameters  : None
Return      : Note
--------------------------------------------------------------------------------*/
void MPU6050_Init(unsigned char ACC_SCALE_CONFIG, unsigned char GYRO_SCALE_CONFIG)
{

    for (;;)
      {
        unsigned char res;

        /*
          First take it out of sleep mode (writes seem to not stick until we take it
          out of sleep mode). Then issue a reset to get a well-defined starting state
          (and go out of sleep mode again).
        */
        //write_mpu6050_reg(MPU6050_REG_PWR_MGMT_1, 0x02);
        I2C_WriteByte(0x02, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1);
        __delay_cycles(1000);
        res = I2C_ReadByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1);
        if (res != 0x02)
          continue;
        __delay_cycles(1000);
        I2C_WriteByte(0x82, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1);
        __delay_cycles(1000);
        res = I2C_ReadByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1);
        if (res != 0x40)
          continue;
        I2C_WriteByte(0x02, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1);
        __delay_cycles(1000);
        res = I2C_ReadByte(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1);
        if (res != 0x02)
          continue;
        __delay_cycles(1000);
        res = I2C_ReadByte(MPU6050_ADDRESS,MPU6050_WHO_AM_I);

        /* Disable digital low-pass filter (DLPF) */
        I2C_WriteByte(0x00, MPU6050_ADDRESS, MPU6050_CONFIG);
        //write_mpu6050_reg(MPU6050_REG_CONFIG, 0);
        /* 1000 Hz sample rate. */
        I2C_WriteByte(7, MPU6050_ADDRESS, MPU6050_SMPLRT_DIV);
        //write_mpu6050_reg(MPU6050_REG_SMPRT_DIV, 7);
        /* Lowest resolution, +-2000 degrees / second and +-16g. */
        I2C_WriteByte(3 << 3, MPU6050_ADDRESS, MPU6050_GYRO_CONFIG);
        //write_mpu6050_reg(MPU6050_REG_GYRO_CONFIG, 3 << 3);
        I2C_WriteByte(3 << 3, MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG);
        //write_mpu6050_reg(MPU6050_REG_ACCEL_CONFIG, 3 << 3);
        /* Disable the Fifo (write 0xf8 to enable temp+gyros_accel). */
        I2C_WriteByte(0x00, MPU6050_ADDRESS, MPU6050_FIFO_EN);
       // write_mpu6050_reg(MPU6050_REG_FIFO_EN, 0x00);
        /*
          Interrupt. Active high, push-pull, hold until cleared, cleared only on
          read of status.
        */
        I2C_WriteByte(0x20, MPU6050_ADDRESS, MPU6050_INT_PIN_CFG);
        //write_mpu6050_reg(MPU6050_REG_INT_PIN_CFG, 0x20);
        /* Enable FIFO overflow and data ready interrupts. */
        I2C_WriteByte(0x11, MPU6050_ADDRESS, MPU6050_INT_ENABLE);
        //write_mpu6050_reg(MPU6050_REG_INT_ENABLE, 0x11);
        /* Disable the FIFO and external I2C master mode. */
        I2C_WriteByte(0x00, MPU6050_ADDRESS, MPU6050_USER_CTRL);
        //write_mpu6050_reg(MPU6050_REG_USER_CTRL, 0x00);

        break;
      }



        __delay_cycles(10000);
}
/*--------------------------------------------------------------------------------
Function    : MPU6050_CheckI2C
Purpose     : Check I2C communication
Parameters  : None
Return      : Value of WHO_AM_I registor (0x68)
-------------------------------------------------------------------------------*/
unsigned char MPU6050_CheckI2C(void)
{
        return I2C_ReadByte(MPU6050_ADDRESS, MPU6050_WHO_AM_I);
}

/*--------------------------------------------------------------------------------
Function    : MPU6050_TestRegConfig
Purpose     : Check the config of some register:
- MPU6050_RA_SMPLRT_DIV == 0x01:
-  MPU6050_RA_CONFIG == 0x03;
- MPU6050_RA_GYRO_CONFIG == 0x01;
- MPU6050_RA_ACCEL_CONFIG == 0x00;
Parameters  : None
Return      : 0 if pass and 1 if failt
-------------------------------------------------------------------------------*/

unsigned char MPU6050_TestRegConfig(void)
{
        unsigned char byBuff, ret;
        I2C_ReadData(&byBuff, MPU6050_ADDRESS, MPU6050_SMPLRT_DIV,1);
        ret =  (byBuff == 0x01 ? 0: 1);
        __delay_cycles(10000);

        I2C_ReadData(&byBuff, MPU6050_ADDRESS, MPU6050_CONFIG,1);
        ret =  (byBuff == 0x03 ? 0: 1);
        __delay_cycles(10000);


        I2C_ReadData(&byBuff, MPU6050_ADDRESS, MPU6050_GYRO_CONFIG,1);
        ret =  (byBuff == 0x01 ? 0: 1);
        __delay_cycles(10000);

        I2C_ReadData(&byBuff, MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG,1);
        ret =  (byBuff == 0x00 ? 0: 1);
        __delay_cycles(10000);

        I2C_ReadData(&byBuff, MPU6050_ADDRESS, MPU6050_PWR_MGMT_1,1);
        ret =  (byBuff == 0x00 ? 0: 1);
        __delay_cycles(10000);

        return ret;
}

/*--------------------------------------------------------------------------------
Function    : MPU6050_GetAccValueRaw
Purpose     : Get raw value x, y, z of accel
Parameters  : PACC_DATA_RAW - pointer to a struct store acc raw data
Return      : NULL
--------------------------------------------------------------------------------*/
void MPU6050_GetAccValueRaw(void * pValue)
{
        unsigned char pBuff[6];
        I2C_ReadData(pBuff, MPU6050_ADDRESS, MPU6050_ACCEL_XOUT_H, 6);

        ((PACC_DATA_RAW)pValue)->x = (int16_t)(CONVERT_TO_16BIT(pBuff[1], pBuff[0]) - Acc_OffsetValueX);
        ((PACC_DATA_RAW)pValue)->y = (int16_t)(CONVERT_TO_16BIT(pBuff[3], pBuff[2]) - Acc_OffsetValueY);
        ((PACC_DATA_RAW)pValue)->z = (int16_t)(CONVERT_TO_16BIT(pBuff[5], pBuff[4]) - Acc_OffsetValueZ);
}

/*--------------------------------------------------------------------------------
Function    : MPU6050_AccConvertData
Purpose     : Scaled data to radian value
Parameters  : ACC_DATA_RAW , void * scaledData
Return      : NULL
--------------------------------------------------------------------------------*/

void MPU6050_AccConvertData(ACC_DATA_RAW rawValue, void * scaledData)
{
        ((PACC_DATA_SCALED)scaledData)->x = (float)rawValue.x / Acc_scaleValue;
        ((PACC_DATA_SCALED)scaledData)->y = (float)rawValue.y / Acc_scaleValue;
        ((PACC_DATA_SCALED)scaledData)->z = (float)rawValue.z / Acc_scaleValue;

}


/*--------------------------------------------------------------------------------
Function    : MPU6050_GetGyroValueRaw
Purpose     : Get raw value x, y, z of Gyro
Parameters  : PGYRO_DATA_RAW - pointer to struct store Gyro data
Return      : NULL
--------------------------------------------------------------------------------*/
void MPU6050_GetGyroValueRaw(void * pValue)
{
        unsigned char pBuff[6];
        I2C_ReadData(pBuff, MPU6050_ADDRESS, MPU6050_GYRO_XOUT_H, 6);

        ((PGYRO_DATA_RAW)pValue)->x = (int16_t)(CONVERT_TO_16BIT(pBuff[1], pBuff[0]) - Gyro_OffsetValueX);
        ((PGYRO_DATA_RAW)pValue)->y = (int16_t)(CONVERT_TO_16BIT(pBuff[3], pBuff[2]) - Gyro_OffsetValueY);
        ((PGYRO_DATA_RAW)pValue)->z = (int16_t)(CONVERT_TO_16BIT(pBuff[5], pBuff[4]) - Gyro_OffsetValueZ);

}

/*--------------------------------------------------------------------------------
Function    : PMU6050_GetGyroValueAngle
Purpose     : Get value x, y, z of Gyro
Parameters  : PGYRO_DATA_ANGLE - pointer to struct store Gyro data
Return      : NULL
--------------------------------------------------------------------------------*/
void PMU6050_GetGyroValueAngle(void * pValue)
{
        ;
}

/*--------------------------------------------------------------------------------
Function    : PMU6050_GyroConvertData
Purpose     : Convert data to m/s^2 by divide to scale
Parameters  : GYRO_DATA_RAW rawValue, void * scaledData
Return      : NULL
--------------------------------------------------------------------------------*/
void PMU6050_GyroConvertData(GYRO_DATA_RAW rawValue, void * scaledData)
{
        ((PGYRO_DATA_SCALED)scaledData)->x = (float)rawValue.x / Gyro_scaleValue;
        ((PGYRO_DATA_SCALED)scaledData)->y = (float)rawValue.y / Gyro_scaleValue;
        ((PGYRO_DATA_SCALED)scaledData)->z = (float)rawValue.z / Gyro_scaleValue;
}

/*--------------------------------------------------------------------------------
Function    :  MPU6050_Calibrate_Gyro
Purpose     :  Get the value to calibrate Gyro
Parameters  : NULL
Return      : NULL
--------------------------------------------------------------------------------*/

void MPU6050_Calibrate_Gyro(void)
{
        int i;
        int x = 0;
        int y = 0;
        int z = 0;
        unsigned char pBuff[6];

        Gyro_OffsetValueX = 0;
        Gyro_OffsetValueY = 0;
        Gyro_OffsetValueZ = 0;
        for(i = 0; i < 2000; i++)
        {

                I2C_ReadData(pBuff, MPU6050_ADDRESS, MPU6050_GYRO_XOUT_H, 6);

                x = (int16_t)(CONVERT_TO_16BIT(pBuff[1], pBuff[0]));
                y = (int16_t)(CONVERT_TO_16BIT(pBuff[3], pBuff[2]));
                z = (int16_t)(CONVERT_TO_16BIT(pBuff[5], pBuff[4]));

                Gyro_OffsetValueX = (Gyro_OffsetValueX + x) >> 1;
                Gyro_OffsetValueY = (Gyro_OffsetValueY + y) >> 1;
                Gyro_OffsetValueZ = (Gyro_OffsetValueZ + z) >> 1;
        }
//        Gyro_OffsetValueX = (int16_t)(SumGyroX/200);
//        Gyro_OffsetValueY = (int16_t)(SumGyroY/200);
//        Gyro_OffsetValueZ = (int16_t)(SumGyroZ/200);
}




/*--------------------------------------------------------------------------------
Function    : ComplementaryFilter
Purpose     : Calculate angle use both acc and gyro
Parameters  : Raw data (ADC 16bit) of acc and gyro
Return      : ANGLE
--------------------------------------------------------------------------------

void Complementary_Filter(ACC_DATA_RAW accData, GYRO_DATA_RAW gyroData, void * pAngle)
{
        float pitchAcc, rollAcc;

        /* Angle around the X axis
        ((PANGLE)pAngle)->x += ((float)gyroData.x / Gyro_scaleValue)*dt; // dt define as 0.015s
        /* Angle around the Y axis
        ((PANGLE)pAngle)->y += ((float)gyroData.y / Gyro_scaleValue)*dt;

        // Compensate for drift with accelerometer data if !bullshit
        // Sensitivity = -2 to 2 G at 16Bit -> 2G = 32768 && 0.5G = 8192
        long forceMagnitudeApprox = ABS(accData.x) + ABS(accData.y) + ABS(accData.z);
        if (forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
        {
                // Turning around the X axis results in a vector on the Y-axis
                //pitchAcc = 57.296 * atan2((float)accData.y, sqrt((float)accData.z*(float)accData.z + (float)accData.x*(float)accData.x));
                ((PANGLE)pAngle)->x = ((PANGLE)pAngle)->x * 0.98 + pitchAcc * 0.02;

                // Turning around the Y axis results in a vector on the X-axis
                //rollAcc = 57.296 * atan2((float)accData.x, sqrt((float)accData.z*(float)accData.z + (float)accData.y*(float)accData.y));
                ((PANGLE)pAngle)->y = ((PANGLE)pAngle)->y* 0.98 + rollAcc * 0.02;
        }

}
*/


