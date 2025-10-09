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
        case CMD_BOARD_STATUS_RESPONSE: {
            proto_board_status_t *board_status_pkt = (proto_board_status_t *)frame;
            board_status_t *slave = getSlaveStatus(frame->header.addr);
            memcpy(slave, &board_status_pkt->fields.status, sizeof(board_status_pkt->fields));
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
        case CMD_BOARD_STATUS: {
            board_status_t *board_status = getBoardStatus();
            proto_board_status_t board_status_pkt = { 0 };
            board_status_pkt.header.cmd = CMD_BOARD_STATUS_RESPONSE;
            board_status_pkt.header.len = sizeof(board_status_pkt.fields);
            board_status_pkt.fields.status = *board_status;
            memcpy(&response, &board_status_pkt, sizeof(proto_board_status_t));
            return &response;
        }
        default: {
            break;
        }
    }
    return NULL;
}
#endif