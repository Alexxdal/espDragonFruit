#ifndef COMMANDMNG_H
#define COMMANDMNG_H

#include "spi_proto.h"

proto_frame_t *handle_frame_master(const proto_frame_t *frame);

proto_frame_t *handle_frame_slave(const proto_frame_t *frame);

#endif // COMMANDMNG_h