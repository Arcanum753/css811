/*
 * css811_sam.c
 *
 *  Created on: 22 џэт. 2020 у.
 *      Author: sam
 */
#include "i2c.h"
#include "CCS811.h"
#include "CCS811_defs.h"
#include "main.h"
uint8_t CCS811_TestBuffer[10] = {0};


int8_t user_ccs811_read(I2C_HandleTypeDef* _hi2c_ccs810, uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)	{
  HAL_I2C_Mem_Read(_hi2c_ccs810, (uint16_t)id, reg_addr, 1, (uint8_t *)data, len, 100);
  return 0;
}
int8_t user_ccs811_write(I2C_HandleTypeDef* _hi2c_ccs810, uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)	{
  if (HAL_I2C_Mem_Write(_hi2c_ccs810, (uint16_t *)id,(uint16_t *) reg_addr, 1, (uint8_t *)&data, len, 1000) != HAL_OK)
  {	return CCS811_COMM_FAIL;	}
  return CCS811_OK;
}

void user_wait_i2c (I2C_HandleTypeDef* _hi2c_ccs810, uint8_t id)	{
	    while (HAL_I2C_GetState			(_hi2c_ccs810) != HAL_I2C_STATE_READY)    {    }
	    while (HAL_I2C_IsDeviceReady	(_hi2c_ccs810, id, 10, 300) == HAL_TIMEOUT);
	    while (HAL_I2C_GetState			(_hi2c_ccs810) != HAL_I2C_STATE_READY)    {    }
}
