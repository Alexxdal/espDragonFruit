#include "driver/gpio.h"
#include "i2c.h"

#if defined(BOARD_MASTER)
#include "driver/i2c_master.h"
#else
#include "driver/i2c_slave.h"
#endif

static const char *TAG = "I2C";


esp_err_t i2c_init(void)
{
#if defined(BOARD_MASTER)
    /* Init Data Ready input */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SPI_S1_CS) | (1ULL << SPI_S2_CS) | (1ULL << SPI_S3_CS),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_HZ,
        .clk_flags = 0
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    return i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
#else
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DATA_READY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(DATA_READY_PIN, 1);

    i2c_config_t conf = {
        .mode = I2C_MODE_SLAVE,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = SLAVE_ADDR
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    return i2c_driver_install(I2C_PORT, I2C_MODE_SLAVE, SLAVE_RX_BUFF_LEN, SLAVE_TX_BUFF_LEN, 0);
#endif
}

#if defined(BOARD_MASTER)
int i2c_hasSlaveData(void)
{
    if (!gpio_get_level(SPI_S1_CS)){
        vTaskDelay(pdMS_TO_TICKS(1));
        return SLAVE1_ADDR;
    }
    if (!gpio_get_level(SPI_S2_CS)){
        vTaskDelay(pdMS_TO_TICKS(1));
        return SLAVE2_ADDR;  
    } 
    if (!gpio_get_level(SPI_S3_CS)){
        vTaskDelay(pdMS_TO_TICKS(1));
        return SLAVE3_ADDR;
    }
    return 0;
}

esp_err_t i2c_write_to(uint8_t addr, const uint8_t *data, size_t len, TickType_t to_ticks) 
{
    return i2c_master_write_to_device(I2C_PORT, addr, data, len, to_ticks);
}

esp_err_t i2c_read_from(uint8_t addr, uint8_t *data, size_t len, TickType_t to_ticks) 
{
    return i2c_master_read_from_device(I2C_PORT, addr, data, len, to_ticks);
}
#else

static inline void drdy_set(int level)
{ 
    gpio_set_level(DATA_READY_PIN, level); 
}

void i2c_drdy_ready(void) 
{ 
    drdy_set(0); 
}

void i2c_drdy_clear(void) 
{ 
    drdy_set(1); 
}

#endif