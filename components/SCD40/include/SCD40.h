/*                  knihovna pro SCD40
 *      Author: Marcel Juchelka
 *		18.12.2023
 *          MCU: ESP32
 *
 */



#define I2C_SCL_PIN         	22               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		21              /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM          0				/*cislo portu i2c */
#define I2C_MASTER_FREQ_HZ          100000      /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TX_BUF   		2                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF   		2                          /*!< I2C master doesn't need buffer */

// Definice I2C adresy senzoru
#define SCD4X_I2C_ADDRESS 0x62

//vypocer crc
#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF


/* direktivy */

#define SCD4X_START_PERIODIC_MEASUREMENT 0x21B1 // I2C Write, Execution time: -, During meas.: No
#define SCD4X_STOP_PERIODIC_MEASUREMENT 0x3F86 // I2C Write, Execution time: 500 ms, During meas.: Yes
#define SCD4X_READ_MEASUREMENT 0xEC05 // I2C Read, Execution time: 1 ms, During meas.: Yes
#define SCD4X_SET_TEMPERATURE_OFFSET 0x241D // I2C Write, Execution time: 1 ms, During meas.: Yes
#define SCD4X_GET_TEMPERATURE_OFFSET 0x2318 // I2C Read, Execution time: 1 ms, During meas.: Yes
#define SCD4X_SET_SENSOR_ALTITUDE 0x2427 // I2C Write, Execution time: 1 ms, During meas.: Yes
#define SCD4X_GET_SENSOR_ALTITUDE 0x2322 // I2C Read, Execution time: 1 ms, During meas.: Yes
#define SCD4X_PERFORM_FORCED_RECALIBRATION 0x362F // I2C Write, Execution time: 400 ms, During meas.: No
#define SCD4X_SET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0x2416 // I2C Write, Execution time: 1 ms, During meas.: Yes
#define SCD4X_GET_AUTOMATIC_SELF_CALIBRATION_ENABLED 0x2313 // I2C Read, Execution time: 1 ms, During meas.: Yes
#define SCD4X_START_LOW_POWER_PERIODIC_MEASUREMENT 0x21AC // I2C Write, Execution time: -, During meas.: No
#define SCD4X_GET_DATA_READY_STATUS 0xE4B8 // I2C Read, Execution time: 1 ms, During meas.: Yes
#define SCD4X_PERSIST_SETTINGS 0x3615 // I2C Write, Execution time: -, During meas.: No
#define SCD4X_GET_SERIAL_NUMBER 0x3682 // I2C Read, Execution time: 1 ms, During meas.: No
#define SCD4X_PERFORM_SELF_TEST 0x3639 // I2C Write, Execution time: 10000 ms, During meas.: No
#define SCD4X_REINIT 0x3646 // I2C Write, Execution time: 20 ms, During meas.: No
#define SCD4X_FACTORY_RESET 0x3632 // I2C Write, Execution time: 1200 ms, During meas.: No
#define SCD4X_SET_AMBIENT_PRESSURE 0xE000 // I2C Write, Execution time: 1 ms, During meas.: Yes
#define SCD4X_GET_AMBIENT_PRESSURE 0xE000 // I2C Read, Execution time: 1 ms, During meas.: Yes

#define ambient_pressure 101000
#define temperature_offset	2	//stupne

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))


typedef struct __attribute__((packed)){			//
	uint16_t co2;
	uint8_t co2_crc;
	uint16_t temp;
	uint8_t temp_crc;
	uint16_t humid;
	uint8_t humid_crc;
} SCD4_DATA_T;

typedef struct __attribute__((packed)){
	uint16_t command_response;
	uint8_t	 command_response_crc;
} SCD4_RESPONSE_T;

extern esp_err_t my_i2c_config(void);
extern esp_err_t my_i2c_config(void);
extern int scd4_read_measure_data(SCD4_DATA_T *data);
extern int scd4_command_send(uint16_t command);
extern int get_data_ready_status(SCD4_RESPONSE_T *data);
extern double calculate_temperature(uint16_t raw_value);
extern double calculate_humidity(uint16_t raw_value);
extern int scd4_get_command_data (uint16_t command, SCD4_RESPONSE_T *data, uint8_t length, uint32_t duration);
extern int scd4_send_command_data(uint16_t command, SCD4_RESPONSE_T *data, uint8_t length, uint32_t duration);
