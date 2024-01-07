/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "SCD40.h"


void app_main(void) {
	printf("******start test CO2   *******!\n");

	my_i2c_config();
	SCD4_DATA_T data = {0};
	SCD4_RESPONSE_T resp_data = {0};
	int ret = -1;

	scd4_command_send(SCD4X_STOP_PERIODIC_MEASUREMENT);
	vTaskDelay(50);
	resp_data.command_response = ambient_pressure/100;
	ret = scd4_send_command_data(SCD4X_SET_AMBIENT_PRESSURE, &resp_data, sizeof(resp_data), 1000);
	if (ret == ESP_OK) printf ("tlak vzduchu %d zapsan\n", resp_data.command_response);
	vTaskDelay(1);
//	resp_data.command_response = (uint16_t)(temperature_offset *(65535.0 /175.0) +0.5);
//	ret = scd4_send_command_data(SCD4X_SET_TEMPERATURE_OFFSET, &resp_data, sizeof(resp_data), 1000);
//	if (ret == ESP_OK) printf ("temp offset %f zapsan\n", (uint16_t)resp_data.command_response * 175.0 / 65535.0);

	ret = scd4_get_command_data(SCD4X_GET_AMBIENT_PRESSURE, &resp_data, sizeof(resp_data), 1000);
	if( ret == ESP_OK) printf("nastaveny tlak je %d\n", resp_data.command_response);
	else printf ("chyba nacteni tlaku %d \n", ret);
	vTaskDelay(1);
	ret = scd4_get_command_data(SCD4X_GET_TEMPERATURE_OFFSET, &resp_data, sizeof(resp_data), 1000);
	if( ret == ESP_OK) printf("nastaveny teplotni offset je %0.2f\n", (double)resp_data.command_response * 175.0 / 65535.0);				//uint16_t word0 = (uint16_t)((T_offset * (65535.0 / 175.0)) + 0.5); // Přidáme 0.5 pro zaokrouhlení
	else printf ("chyba nacteni ofsetu %d \n", ret);
	vTaskDelay(1);

//	vTaskDelay(200);




	while (1) {
		scd4_command_send(SCD4X_START_PERIODIC_MEASUREMENT);
		vTaskDelay(501);

//		do{
//			ret = scd4_get_command_data(SCD4X_GET_DATA_READY_STATUS, &resp_data, sizeof(resp_data), 1000);
////			printf("ready status = %i data = %X \t crc %X \n", ret, resp_data.command_response , resp_data.command_response_crc );
//			if(resp_data.command_response & 0x07FF) break;
//			vTaskDelay(500);
//		}while(1);
		int ret = scd4_read_measure_data(&data);
		printf("co2 = %d \t temp = %f \t humidy = %f \t error %d \n", data.co2, calculate_temperature(data.temp), calculate_humidity(data.humid), ret);
		vTaskDelay(1);
		scd4_command_send(SCD4X_STOP_PERIODIC_MEASUREMENT);
		vTaskDelay(5400);
//		ret = scd4_get_command_data(SCD4X_GET_TEMPERATURE_OFFSET, &resp_data, sizeof(resp_data), 1000);
//		if( ret == ESP_OK) printf("nastaveny teplotni offset je %0.2f\n", (double)resp_data.command_response * 175.0 / 65535.0);				//uint16_t word0 = (uint16_t)((T_offset * (65535.0 / 175.0)) + 0.5); // Přidáme 0.5 pro zaokrouhlení
//		else printf ("chyba nacteni ofsetu %d \n", ret);
//

	}
}
