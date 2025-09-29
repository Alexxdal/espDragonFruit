#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"

#include "i2c.h"
#include "i2c_proto.h"

static const char *TAG = "I2C_PROTO";

/* ========================= MASTER SIDE ========================= */

#if defined(BOARD_MASTER)

esp_err_t i2c_send_cmd(uint8_t addr, uint8_t cmd, const uint8_t *payload, uint8_t len, TickType_t to)
{
    if (len > I2C_MAX_PAYLOAD) return ESP_ERR_INVALID_SIZE;

    uint8_t buf[2 + I2C_MAX_PAYLOAD];
    buf[0] = cmd;
    buf[1] = len;
    if (len && payload) memcpy(&buf[2], payload, len);

    esp_err_t e = i2c_write_to(addr, buf, (size_t)(2 + len), to);
    if (e != ESP_OK) {
        ESP_LOGW(TAG, "send_cmd addr=0x%02X cmd=0x%02X len=%u -> err=%d", addr, cmd, (unsigned)len, e);
    }
    return e;
}

esp_err_t i2c_recv_resp(uint8_t addr, uint8_t *buf, uint8_t bufsize, uint8_t *out_len, TickType_t to)
{
    // Legge prima l'header (echo_cmd, len), poi il payload se len>0
    uint8_t hdr[2] = {0};
    esp_err_t e = i2c_read_from(addr, hdr, 2, to);
    if (e != ESP_OK) {
        ESP_LOGW(TAG, "recv_resp hdr addr=0x%02X -> err=%d", addr, e);
        return e;
    }

    uint8_t rlen = hdr[1];
    if (rlen > bufsize) {
        ESP_LOGE(TAG, "recv_resp len overflow: rlen=%u > bufsize=%u", rlen, bufsize);
        return ESP_ERR_NO_MEM;
    }

    if (rlen) {
        e = i2c_read_from(addr, buf, rlen, to);
        if (e != ESP_OK) {
            ESP_LOGW(TAG, "recv_resp payload addr=0x%02X len=%u -> err=%d", addr, rlen, e);
            return e;
        }
    }
    if (out_len) *out_len = rlen;

    ESP_LOGD(TAG, "recv_resp OK addr=0x%02X echo_cmd=0x%02X len=%u", addr, hdr[0], rlen);
    return ESP_OK;
}

#endif /* BOARD_MASTER */

/* ========================= SLAVE SIDE ========================= */

#if defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)

// Buffer TX condiviso (driver slave lo copia internamente)
static uint8_t s_tx_buf[2 + I2C_MAX_PAYLOAD];

static inline void make_resp(uint8_t cmd, const uint8_t *payload, uint8_t len)
{
    if (len > I2C_MAX_PAYLOAD) len = I2C_MAX_PAYLOAD;
    s_tx_buf[0] = (uint8_t)(cmd | 0x80);
    s_tx_buf[1] = len;
    if (len && payload) memcpy(&s_tx_buf[2], payload, len);
}

static void handle_command(const uint8_t *rx, int n)
{
    if (n < 2) {
        // frame minimo non valido
        make_resp(0x7F, NULL, 0);
        return;
    }
    const uint8_t cmd = rx[0];
    const uint8_t len = rx[1];

    if ((int)(2 + len) > n || len > I2C_MAX_PAYLOAD) {
        // lunghezza incoerente
        make_resp(0x7E, NULL, 0);
        return;
    }

    const uint8_t *pl = &rx[2];

    switch (cmd) {
        case CMD_PING: {
            static const char pong[] = "PONG";
            make_resp(CMD_PING, (const uint8_t*)pong, sizeof(pong));
            break;
        }
        case CMD_WIFI_ON: {
            // TODO: accendi davvero il Wi-Fi qui (non bloccare!)
            make_resp(CMD_WIFI_ON, NULL, 0);
            break;
        }
        case CMD_WIFI_SCAN: {
            // TODO: lancia una scansione vera in task separato; qui demo
            static const char demo[] = "ssid:demo,rssi:-42";
            make_resp(CMD_WIFI_SCAN, (const uint8_t*)demo, sizeof(demo));
            break;
        }
        default: {
            make_resp(0x00, NULL, 0);  // comando sconosciuto
            break;
        }
    }
}

void i2c_slave_task(void *arg)
{
    (void)arg;
    uint8_t rx[2 + I2C_MAX_PAYLOAD] = { 0 };

    // All'avvio non abbiamo nulla da dire: DRDY idle (alto se active-low)
    i2c_drdy_clear();

    for (;;) {
        // 1) Attendi un frame in ingresso dal master (WRITE del master)
        //    Timeout 1000 ms per evitare lock perenne
        int n = i2c_slave_read_buffer(I2C_PORT, rx, sizeof(rx), pdMS_TO_TICKS(1000));
        if (n <= 0) {
            // niente ricevuto in finestra: loop
            continue;
        }

        // Abbassa DRDY a idle (alto) quando arriva un nuovo comando:
        // così, se il master non ha fatto ancora la READ precedente, non restiamo "pronti" per sempre.
        i2c_drdy_clear();

        // 2) Elabora il comando e prepara la risposta nel buffer TX
        handle_command(rx, n);

        // 3) Esponi la risposta allo slave driver (verrà servita alla prossima READ del master)
        size_t tx_len = (size_t)(2 + s_tx_buf[1]);
        esp_err_t e = i2c_slave_write_buffer(I2C_PORT, s_tx_buf, tx_len, pdMS_TO_TICKS(1000));
        if (e <= 0) {
            // Se non scrive, ritenta al prossimo giro
            ESP_LOGW(TAG, "slave_write_buffer failed (%d), tx_len=%u", e, (unsigned)tx_len);
            continue;
        }

        // 4) Segnala al master che i dati sono pronti (active-low → 0 = ready)
        i2c_drdy_ready();
        // DRDY tornerà a idle alla prossima ricezione comando (vedi inizio ciclo)
    }
}

#endif /* any SLAVE */