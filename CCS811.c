#include "i2c.h"
#include "CCS811.h"
#include "CCS811_defs.h"

int8_t ccs811_init(struct ccs811_dev *dev)
{
    int8_t rslt;
    uint8_t data[4] = {0x11, 0xE5, 0x72, 0x8A}; //Reset key

    /* chip id read try count */
    uint8_t try_count = 5;
    uint8_t chip_id = 0;

    while (try_count)
    {
    	dev->wait_i2c(dev->hi2c_ccs810, dev->dev_id);
        rslt = dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_HW_ID_ADDR, &chip_id, 1);

        /* Check for chip id validity */
        if ((rslt == CCS811_OK) && (chip_id == CSS811_HW_ID))
        {
            dev->chip_id = chip_id;
            // sw reset

            rslt = dev->write(dev->hi2c_ccs810,dev->dev_id, CSS811_SW_RESET, data, 4);
//            dev->delay_ms(100);
            dev->wait_i2c(dev->hi2c_ccs810,dev->dev_id);
            /* Start sensor */

            rslt = dev->write(dev->hi2c_ccs810,dev->dev_id, CSS811_APP_START_ADDR, 0, 0);
//            dev->delay_ms(70);
            dev->wait_i2c(dev->hi2c_ccs810,dev->dev_id);

            if (rslt == CCS811_OK)
            {
                /* Set mode */
                rslt = dev->write(dev->hi2c_ccs810,dev->dev_id, CSS811_MEAS_MODE_ADDR, dev->settings.mode, 1);
                rslt = dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_MEAS_MODE_ADDR, &chip_id, 1);
            }
            break;
        }

        dev->wait_i2c(dev->hi2c_ccs810,dev->dev_id);
        --try_count;
    }

    /* Chip id check failed */
    if (!try_count)
    {
        rslt = 5; 
    }

    return rslt;
}

int8_t ccs811_get_data(struct ccs811_dev *dev, struct ccs811_data *data)
{
    int8_t rslt = 0;
    bool dataReady = false;
    uint32_t tVOC = 0;
    uint32_t CO2 = 0;

    //! Debug Variables
    static uint32_t datareadycntr = 0;
    static uint32_t tryReadCntr = 0;
    //!

    /* Array to store the eco2, tvoc  */
    uint8_t alg_data[4] = {0};

    //! Debug Variables
    tryReadCntr++;

    /* Read the pressure and temperature data from the sensor */

    dataReady = isDataReady(dev);

    if ((dataReady))	    {
        rslt = dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_ALG_RESULT_ADDR, alg_data, 4);

        uint8_t co2MSB = alg_data[0];
        uint8_t co2LSB = alg_data[1];
        uint8_t tvocMSB = alg_data[2];
        uint8_t tvocLSB = alg_data[3];
        CO2 = ((uint32_t)co2MSB << 8) | co2LSB;
        tVOC = ((uint32_t)tvocMSB << 8) | tvocLSB;
        data->eCO2 = CO2;
        data->VOCS = tVOC;

        //! Debug Variables
        datareadycntr++;
        /* Parse the read data from the sensor */
    }

//    dev->delay_ms(1);
    dev->wait_i2c(dev->hi2c_ccs810,dev->dev_id);
    return rslt;
}

bool ccs811_check_error(struct ccs811_dev *dev)
{
//    uint8_t reg_data[4] = {0};
    uint8_t statusReg = 0;
    uint8_t errorId = 0;
    bool isErrorClear = true;
    static uint32_t errorCounter = 0;

    dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_STATUS_ADDR, &statusReg, 1);
    if (((statusReg & 0x1) << 0) == 1)
    {
        dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_ERROR_ID_ADDR, &errorId, 1);
        isErrorClear = false;
        errorCounter++;
    }

    return isErrorClear;
}
bool isDataReady(struct ccs811_dev *dev)
{
    bool dataReady = false;
    uint8_t rslt = 0;
    uint8_t status = 0;

    rslt = dev->read(dev->hi2c_ccs810,dev->dev_id, CSS811_STATUS_ADDR, &status, 1);
    if (ccs811_check_error(dev))
    {
        dataReady = (status >> 3) & (0x1);
    }

    return dataReady;
}


void css811_setEnvironmentalData(struct ccs811_dev *dev)
{
    int rH = dev->comp_data.humidity * 1000; //42.348 becomes 42348
    int temp = dev->comp_data.temperature * 1000;    //23.2 becomes 23200

    uint8_t envData[4];

    //Split value into 7-bit integer and 9-bit fractional
    envData[0] = ((rH % 1000) / 100) > 7 ? (rH / 1000 + 1) << 1 : (rH / 1000) << 1;
    envData[1] = 0; //CCS811 only supports increments of 0.5 so bits 7-0 will always be zero
    if (((rH % 1000) / 100) > 2 && (((rH % 1000) / 100) < 8))
    {
        envData[0] |= 1; //Set 9th bit of fractional to indicate 0.5%
    }

    temp += 25000; //Add the 25C offset
    //Split value into 7-bit integer and 9-bit fractional
    envData[2] = ((temp % 1000) / 100) > 7 ? (temp / 1000 + 1) << 1 : (temp / 1000) << 1;
    envData[3] = 0;
    if (((temp % 1000) / 100) > 2 && (((temp % 1000) / 100) < 8))
    {
        envData[2] |= 1; //Set 9th bit of fractional to indicate 0.5C
    }

    dev->write(dev->hi2c_ccs810,dev->dev_id, 		CSS811_ENV_DATA, envData, sizeof(envData));
    dev->wait_i2c(dev->hi2c_ccs810,dev->dev_id);
}
