#include "touch.h"

extern SPI_HandleTypeDef TOUCH_SPI;

static void spi_set_speed(uint32_t speed){
	__HAL_SPI_DISABLE(&TOUCH_SPI);
	TOUCH_SPI.Init.BaudRatePrescaler = speed;
	HAL_SPI_Init(&TOUCH_SPI);
}
static uint16_t touch_read(uint8_t cmd){
	uint8_t tx[3], rx[3];
	uint16_t temp;
	tx[0] = cmd;
	tx[1] = 0x00;
	tx[2] = 0x00;
	HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&TOUCH_SPI, tx, rx, 3, TOUCH_SPI_TIMEOUT);
	HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET);
	temp = (rx[1] << 8) | rx[2];
	temp =(temp >> 3);
	return temp;
}
static uint16_t touch_map(uint16_t raw, uint16_t min, uint16_t max, uint16_t out){
	uint16_t product;
	if(raw < min) raw = min;
	if(raw > max) raw = max;
	product = (max - raw) * (out - 1) / (max - min);
	return product;
}

uint8_t touch_get(uint16_t *x, uint16_t *y){
	uint16_t raw_x = 0;
	uint16_t raw_y = 0;
	uint16_t raw_z;
	spi_set_speed(SPI_TOUCH);
	raw_z = touch_read(XPT_CMD_Z);
	if (raw_z < TOUCH_THRESHOLD){
		spi_set_speed(SPI_SCREEN);
		return 0;
	}

	raw_x = touch_read(XPT_CMD_X);
	raw_y = touch_read(XPT_CMD_Y);
	spi_set_speed(SPI_SCREEN);

	*x = touch_map(raw_y, TOUCH_Y_MIN, TOUCH_Y_MAX, TOUCH_OUT_W);
	*y = touch_map(raw_x, TOUCH_X_MIN, TOUCH_X_MAX, TOUCH_OUT_H);
	return 1;
}
