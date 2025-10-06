#include "driver/gpio.h"
#include "i2c.h"
#include "log.h"

#if defined(BOARD_MASTER)
static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev_s1, s_dev_s2, s_dev_s3;
static int SLAVE1_RDY = false;
static int SLAVE2ù_RDY = false;
static int SLAVE3_RDY = false;
static void IRAM_ATTR slave1_data_ready_isr(void *args)
{
    SLAVE1_RDY = true;
    return;
}
static void IRAM_ATTR slave2_data_ready_isr(void *args)
{
    SLAVE2_RDY = true;
    return;
}
static void IRAM_ATTR slave3_data_ready_isr(void *args)
{
    SLAVE3_RDY = true;
    return;
}
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
        .intr_type = GPIO_INTR_NEGEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = 1,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_bus));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .scl_speed_hz = I2C_HZ,
    };
    dev_cfg.device_address = SLAVE1_ADDR; 
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev_s1));
    dev_cfg.device_address = SLAVE2_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev_s2));
    dev_cfg.device_address = SLAVE3_ADDR;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev_s3));
    /* Install gpio ISR */
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(SPI_S1_CS, slave1_data_ready_isr, (void *)SPI_S1_CS));
    ESP_ERROR_CHECK(gpio_isr_handler_add(SPI_S2_CS, slave2_data_ready_isr, (void *)SPI_S2_CS));
    ESP_ERROR_CHECK(gpio_isr_handler_add(SPI_S3_CS, slave3_data_ready_isr, (void *)SPI_S3_CS));

    log_message(LOG_LEVEL_INFO, TAG, "Master I2C Bus initialized");
    return ESP_OK;
#else
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DATA_READY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
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
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, I2C_MODE_SLAVE, SLAVE_RX_BUFF_LEN, SLAVE_TX_BUFF_LEN, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1));
    log_message(LOG_LEVEL_INFO, TAG, "Slave I2C Bus initialized");
    return ESP_OK;
#endif
}

#if defined(BOARD_MASTER)
// int i2c_hasSlaveData(void)
// {
//     if (!gpio_get_level(SPI_S1_CS)){
//         return SLAVE1_ADDR;
//     }
//     if (!gpio_get_level(SPI_S2_CS)){
//         return SLAVE2_ADDR;  
//     } 
//     if (!gpio_get_level(SPI_S3_CS)){
//         return SLAVE3_ADDR;
//     }
//     return 0;
// }
int i2c_hasSlaveData(void)
{
    if (SLAVE1_RDY){
        SLAVE1_RDY = false;
        return SLAVE1_ADDR;
    }
    if (SLAVE2_RDY){
        SLAVE2_RDY = false;
        return SLAVE2_ADDR;  
    } 
    if (SLAVE3_RDY){
        SLAVE3_RDY = false;
        return SLAVE3_ADDR;
    }
    return 0;
}

static inline i2c_master_dev_handle_t dev_for(uint8_t addr)
{
    if (addr == SLAVE1_ADDR) return s_dev_s1;
    if (addr == SLAVE2_ADDR) return s_dev_s2;
    if (addr == SLAVE3_ADDR) return s_dev_s3;
    return NULL;
}

esp_err_t i2c_write_to(uint8_t addr, const uint8_t *data, size_t len, TickType_t to_ticks)
{
    i2c_master_dev_handle_t h = dev_for(addr);
    return h ? i2c_master_transmit(h, data, len, to_ticks) : ESP_ERR_NOT_FOUND;
}

esp_err_t i2c_read_from(uint8_t addr, uint8_t *data, size_t len, TickType_t to_ticks)
{
    i2c_master_dev_handle_t h = dev_for(addr);
    return h ? i2c_master_receive(h, data, len, to_ticks) : ESP_ERR_NOT_FOUND;
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