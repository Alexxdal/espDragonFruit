#include <string.h>
#include "commandMng.h"
#include "log.h"
#include "board.h"

static const char *TAG = "COMMAND_MNG";

#if defined(BOARD_MASTER)
proto_frame_t *handle_frame_master(const proto_frame_t *frame)
{
    //log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from slave=%d", frame->header.cmd, frame->header.len, frame->header.addr);

    switch (frame->header.cmd) {
        case CMD_CHIP_INFO_RESPONSE: {
            proto_chip_info_t *chip_info = (proto_chip_info_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            memcpy(&slave->chip, &chip_info->fields.chip, sizeof(chip_info->fields.chip));
            log_message(LOG_LEVEL_DEBUG, TAG, "Chip model: %d", slave->chip.model);
            break;
        }
        case CMD_WIFI_SCAN: {
            break;
        }
        default: {
            break;
        }
    }
    return NULL;
}
#elif defined(BOARD_SLAVE1) || defined(BOARD_SLAVE2) || defined(BOARD_SLAVE3)
proto_frame_t *handle_frame_slave(const proto_frame_t *frame)
{
    static proto_frame_t response = { 0 };
    //log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from master", frame->header.cmd, frame->header.len);

    switch (frame->header.cmd) {
        case CMD_POLL: {
            response.header.cmd = CMD_ACK;
            response.header.len = 0;
            return &response;
        }
        case CMD_CHIP_INFO: {
            board_status_t *board_status = getBoardStatus();
            proto_chip_info_t chip_info = {
                .header.cmd = CMD_CHIP_INFO_RESPONSE,
                .fields.chip = board_status->chip
            };
            memcpy(&response, &chip_info, sizeof(proto_chip_info_t));
            return &response;
        }
        case CMD_WIFI_SCAN: {
            break;
        }
        default: {
            break;
        }
    }
    return NULL;
}
#endif