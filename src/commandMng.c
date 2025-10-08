#include "commandMng.h"
#include "log.h"

static const char *TAG = "COMMAND_MNG";

proto_frame_t *handle_frame(const proto_frame_t *frame)
{
    static proto_frame_t response = { 0 };
    #if defined(BOARD_MASTER)
    log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from slave=%d", frame->header.cmd, frame->header.len, frame->header.addr);
    #else
    log_message(LOG_LEVEL_DEBUG, TAG, "Received command=%d with len=%d from master", frame->header.cmd, frame->header.len);
    #endif

    switch (frame->header.cmd) {
        case CMD_PING: {
            response.header.cmd = CMD_PONG;
            response.header.len = 0;
            return &response;
        }
        case CMD_WIFI_ON: {
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