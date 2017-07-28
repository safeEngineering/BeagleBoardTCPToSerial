#ifndef SERIAL_PACKET_HPP
#define SERIAL_PACKET_HPP

#include <iostream>

namespace SafeEngineering
{
    namespace Comm
    {
        class SerialPacket
        {
        public:
            SerialPacket(const uint8_t* pPacket, size_t len)
            {
                // Truncate tail if data length is over SERIAL_PACKET_LENGTH
                if(len <= SERIAL_PACKET_LENGTH)
                    m_packetLen = len;
                else
                    m_packetLen = SERIAL_PACKET_LENGTH;                    
                // Copy data to internal buffer
                memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                memcpy(m_packet, pPacket, m_packetLen);
            }
            
            const uint8_t* Data() const
            {
                return m_packet;
            }

            uint8_t Length() const
            {
                return m_packetLen;
            }

        private:
            // Data packet from external device
            uint8_t m_packet[SERIAL_PACKET_LENGTH + 1];
            uint8_t m_packetLen;            
        };
        
    }   // namespace Comm
}

#endif // SERIAL_PACKET_HPP