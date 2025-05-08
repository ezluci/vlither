#ifndef CALLBACK_H
#define CALLBACK_H

#include "../external/mg/mongoose.h"

void client_callback(struct mg_connection* c, int ev, void* ev_data);

void gotPacket(struct mg_connection* c, const uint8_t* packet, int packet_len);

#endif