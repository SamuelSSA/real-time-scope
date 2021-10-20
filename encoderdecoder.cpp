#include "encoderdecoder.h"

#include <QDebug>

constexpr char packet_root = 'i';
constexpr char init_of_packet[4] = "ini";


EncoderDecoder::EncoderDecoder(QObject *parent): QObject(parent)
{

}


size_t EncoderDecoder::cobsEncode(const void *data, size_t length, uint8_t *buffer)
{
    assert(data && buffer);

    uint8_t *encode = buffer; // Encoded byte pointer
    uint8_t *codep = encode++; // Output code pointer
    uint8_t code = 1; // Code value

    for (const uint8_t *byte = (const uint8_t *)data; length--; ++byte)
    {
        if (*byte) // Byte not zero, write it
            *encode++ = *byte, ++code;

        if (!*byte || code == 0xff) // Input is zero or block completed, restart
        {
            *codep = code, code = 1, codep = encode;
            if (!*byte || length)
                ++encode;
        }
    }
    *codep = code; // Write final code value

    return (size_t)(encode - buffer);
}

bool EncoderDecoder::decodeData(std::vector<uint8_t> &input, DATA_PACKET &data_packet)
{
    uint8_t *data_ptr = input.data();
    bool ret = false;

    int size = sizeof(DATA_PACKET);
    if(input.size() >= size)
    {
        for(int i = 0; i < input.size(); i++)
        {
            if( (((int8_t)*data_ptr) == packet_root) && (memcmp(init_of_packet, (char *)data_ptr,4) == 0))
            {
                data_packet = *((DATA_PACKET *) data_ptr);
                ret = true;
                break;
            }
            data_ptr++;
        }
    }
    return ret;
}

/* source: https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing */
/** COBS decode data from buffer
    @param buffer Pointer to encoded input bytes
    @param length Number of bytes to decode
    @param data Pointer to decoded output data
    @return Number of bytes successfully decoded
    @note Stops decoding if delimiter byte is found
*/
size_t EncoderDecoder::cobsDecode(const uint8_t *buffer, size_t length, void *data)
{
    assert(buffer && data);

    const uint8_t *byte = buffer; // Encoded input byte pointer
    uint8_t *decode = (uint8_t *)data; // Decoded output byte pointer

    for (uint8_t code = 0xff, block = 0; byte < buffer + length; --block)
    {
        if (block) // Decode block byte
            *decode++ = *byte++;
        else
        {
            if (code != 0xff) // Encoded zero, write it
                *decode++ = 0;
            block = code = *byte++; // Next block length
            if (!code) // Delimiter code found
                break;
        }
    }

    return (size_t)(decode - (uint8_t *)data);
}
