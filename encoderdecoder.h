#ifndef ENCODERDECODER_H
#define ENCODERDECODER_H

#include <QObject>

#include "constants.h"

struct DATA_PACKET
{
    char init_of_packet[4];
    uint8_t code;
    uint8_t reserved[3];
    float data_output[GRAPHS][SAMPLES];
    char end_of_packet[4];
};

struct COMMAND
{
    char cmd_init[4];
    char code[4];
    uint32_t integer_value;
    float float_value;
    char end_of_cmd[4];
};

class EncoderDecoder : public QObject
{
    Q_OBJECT
public:
    EncoderDecoder(QObject *parent = nullptr);

    size_t cobsDecode(const uint8_t *buffer, size_t length, void *data);
    size_t cobsEncode(const void *data, size_t length, uint8_t *buffer);
    bool decodeData(std::vector<uint8_t> &input, DATA_PACKET &data_packet);

signals:
    void decodedData(std::vector<float>);

private:

};

#endif // ENCODERDECODER_H
