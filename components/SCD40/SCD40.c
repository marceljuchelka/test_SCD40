#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include <stdio.h>
#include "SCD40.h"
#include <rom/ets_sys.h>


	/*konfigurace i2c na ESP  vcetne driveru*/
esp_err_t my_i2c_config(void) {

	int i2c_master_port = I2C_MASTER_NUM;
	i2c_config_t i2c_conf = { 	.mode = I2C_MODE_MASTER,
								.sda_io_num = I2C_SDA_PIN,
								.sda_pullup_en = GPIO_PULLUP_ENABLE,
								.scl_io_num = I2C_SCL_PIN,
								.scl_pullup_en = GPIO_PULLUP_ENABLE,
								.master.clk_speed = I2C_MASTER_FREQ_HZ,
								.clk_flags = 0 };

	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
	return i2c_driver_install(i2c_master_port, i2c_conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


/* vypocet crc do dat se promenna do lenght velikost promenne zpet se vraci crc  */
uint8_t calculate_crc8(const uint8_t *data, uint16_t length) {
    uint8_t crc = CRC8_INIT;
    for (uint16_t i = 0; i < length; ++i) {
        crc ^= data[i]; // XOR byte into least sig. byte of crc
        for (uint8_t bit = 8; bit > 0; --bit) { // Loop over each bit
            if (crc & 0x80) { // If the uppermost bit is 1...
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            } else { // Else lowermost bit is 0
                crc = (crc << 1);
            }
        }
    }
    return crc;
}


/* odeslani commandu do senzoru bez dat */
int scd4_command_send(uint16_t command) {
	esp_err_t ret;
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SCD4X_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, command >> 8, I2C_MASTER_ACK); // MSB
	i2c_master_write_byte(cmd, command & 0xFF, I2C_MASTER_ACK); // LSB
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK)
		return ret;
	return 0;
}

int scd4_read_i2c_data(uint8_t *data, uint8_t lenght, uint32_t duration_us){
 	esp_err_t ret;
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SCD4X_I2C_ADDRESS << 1) | I2C_MASTER_READ, I2C_MASTER_ACK);
//	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100);
	i2c_cmd_link_delete(cmd);
	ets_delay_us(duration_us);
	cmd = i2c_cmd_link_create();
	i2c_master_read(cmd, (uint8_t*)data, lenght, I2C_MASTER_LAST_NACK);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100);
	i2c_cmd_link_delete(cmd);
	if(ret != ESP_OK) return ret;
	return 0;
}


double calculate_temperature(uint16_t raw_value) {
    return -45.0 + 175.0 * raw_value / (65536.0 - 1.0);
}

double calculate_humidity(uint16_t raw_value) {
    return 100.0 * raw_value / (65536.0 - 1.0);
}

/* nacteni namerenych hodnot ze senzoru */
int scd4_read_measure_data(SCD4_DATA_T *data){
 	esp_err_t ret;
	ret = scd4_command_send(SCD4X_READ_MEASUREMENT);
	if(ret != ESP_OK) return ret;
	ret = scd4_read_i2c_data((uint8_t*)data, sizeof(*data), 1000);

	uint16_t crc_source;
    crc_source = (data->humid);
    if(calculate_crc8((uint8_t*)&crc_source, 2) != data->humid_crc) return -3;
    crc_source = (data->temp);
    if(calculate_crc8((uint8_t*)&crc_source, 2) != data->temp_crc) return -2;
    crc_source = (data->co2);
    if(calculate_crc8((uint8_t*)&crc_source, 2) != data->co2_crc) return -1;

	data->co2 = SWAP_UINT16(data->co2);
	data->temp = SWAP_UINT16(data->temp);
	data->humid = SWAP_UINT16(data->humid);
//	printf("RAW *** co2 = %X \t co2_crc =%X \t temp = %X \t temp_crc = %X \t humidy = %X \t humid_crc = %X \n\n", data->co2,data->co2_crc, data->temp,data->temp_crc, data->humid, data->humid_crc);


	return ESP_OK;
}

/* nacteni dat po poslani commandu */
int scd4_get_command_data (uint16_t command, SCD4_RESPONSE_T *data, uint8_t length, uint32_t duration){
	esp_err_t ret;

	ret = scd4_command_send(command);
	if(ret != ESP_OK) return -2;
	ret = scd4_read_i2c_data((uint8_t*)data, sizeof(*data), duration);
	if(ret != ESP_OK) return -3;

	uint16_t crc_source = (data->command_response);
	if(calculate_crc8((uint8_t*)&crc_source, 2) != data->command_response_crc) return -1;

	data->command_response = SWAP_UINT16(data->command_response);
    return ESP_OK;
}




/* poslani commandu a dat  */
int scd4_send_command_data(uint16_t command, SCD4_RESPONSE_T *data, uint8_t length, uint32_t duration){
	esp_err_t ret;

	uint16_t crc_source = SWAP_UINT16(data->command_response);
	data->command_response_crc = calculate_crc8((uint8_t*)&crc_source, 2);
	i2c_cmd_handle_t cmd;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SCD4X_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
	i2c_master_write_byte(cmd, command >> 8, I2C_MASTER_ACK); // MSB
	i2c_master_write_byte(cmd, command & 0xFF, I2C_MASTER_ACK); // LSB
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK)
		return ret;
	ets_delay_us(duration);

	cmd = i2c_cmd_link_create();
	i2c_master_write_byte(cmd, (uint8_t)(data->command_response >> 8) ,I2C_MASTER_ACK); // MSB
	i2c_master_write_byte(cmd, (uint8_t)(data->command_response),I2C_MASTER_ACK); // LSB
	i2c_master_write_byte(cmd, data->command_response_crc,I2C_MASTER_ACK);	//CRC
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK)
		return ret;
	return 0;
}


