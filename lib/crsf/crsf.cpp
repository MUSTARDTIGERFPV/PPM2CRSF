#include <Arduino.h>
#include "crsf.h"

uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        crc = crsf_crc8tab[crc ^ *ptr++];
    }
    return crc;
}

void crsfPreparePacket(uint8_t* packet, const uint8_t destination, const uint8_t type, const uint8_t payload_length, const uint8_t payload[])
{
    // Calculate length + type field + CRC8
    uint8_t length = payload_length + 2;
    // Write header
    packet[0] = destination;
    packet[1] = length;
    packet[2] = type;
    // Write payload
    for (uint8_t i = 0; i < payload_length; i++) {
        packet[i + 3] = payload[i];
    }
    // Insert CRC
    packet[length + 1] = crsf_crc8(&packet[2], payload_length + 1);
}

void crsfPrepareChannelsPacket(uint8_t* packet, int channels[])
{
    int mapped_channels[CRSF_MAX_CHANNEL_COUNT];
    for (uint8_t i = 0; i < CRSF_MAX_CHANNEL_COUNT; i++) {
        mapped_channels[i] = map(channels[i],1000,2000,CRSF_CHANNEL_MIN,CRSF_CHANNEL_MAX);
    }
    uint8_t payload[CRSF_TYPE_CHANNELS_PAYLOAD_LENGTH];
    payload[0] = (uint8_t)(mapped_channels[0] & 0x07FF);
    payload[1] = (uint8_t)((mapped_channels[0] & 0x07FF) >> 8 | (mapped_channels[1] & 0x07FF) << 3);
    payload[2] = (uint8_t)((mapped_channels[1] & 0x07FF) >> 5 | (mapped_channels[2] & 0x07FF) << 6);
    payload[3] = (uint8_t)((mapped_channels[2] & 0x07FF) >> 2);
    payload[4] = (uint8_t)((mapped_channels[2] & 0x07FF) >> 10 | (mapped_channels[3] & 0x07FF) << 1);
    payload[5] = (uint8_t)((mapped_channels[3] & 0x07FF) >> 7 | (mapped_channels[4] & 0x07FF) << 4);
    payload[6] = (uint8_t)((mapped_channels[4] & 0x07FF) >> 4 | (mapped_channels[5] & 0x07FF) << 7);
    payload[7] = (uint8_t)((mapped_channels[5] & 0x07FF) >> 1);
    payload[8] = (uint8_t)((mapped_channels[5] & 0x07FF) >> 9 | (mapped_channels[6] & 0x07FF) << 2);
    payload[9] = (uint8_t)((mapped_channels[6] & 0x07FF) >> 6 | (mapped_channels[7] & 0x07FF) << 5);
    payload[10] = (uint8_t)((mapped_channels[7] & 0x07FF) >> 3);
    payload[11] = (uint8_t)((mapped_channels[8] & 0x07FF));
    payload[12] = (uint8_t)((mapped_channels[8] & 0x07FF) >> 8 | (mapped_channels[9] & 0x07FF) << 3);
    payload[13] = (uint8_t)((mapped_channels[9] & 0x07FF) >> 5 | (mapped_channels[10] & 0x07FF) << 6);
    payload[14] = (uint8_t)((mapped_channels[10] & 0x07FF) >> 2);
    payload[15] = (uint8_t)((mapped_channels[10] & 0x07FF) >> 10 | (mapped_channels[11] & 0x07FF) << 1);
    payload[16] = (uint8_t)((mapped_channels[11] & 0x07FF) >> 7 | (mapped_channels[12] & 0x07FF) << 4);
    payload[17] = (uint8_t)((mapped_channels[12] & 0x07FF) >> 4 | (mapped_channels[13] & 0x07FF) << 7);
    payload[18] = (uint8_t)((mapped_channels[13] & 0x07FF) >> 1);
    payload[19] = (uint8_t)((mapped_channels[13] & 0x07FF) >> 9 | (mapped_channels[14] & 0x07FF) << 2);
    payload[20] = (uint8_t)((mapped_channels[14] & 0x07FF) >> 6 | (mapped_channels[15] & 0x07FF) << 5);
    payload[21] = (uint8_t)((mapped_channels[15] & 0x07FF) >> 3);
    crsfPreparePacket(packet, CRSF_ADDR_MODULE, CRSF_TYPE_CHANNELS, CRSF_TYPE_CHANNELS_PAYLOAD_LENGTH, payload);
}
