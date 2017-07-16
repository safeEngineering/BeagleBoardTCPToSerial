#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <asio.hpp>

#include "Utils.hpp"

#define SERIAL_PACKET_LENGTH            8

#define START_OF_PACKET                 0x02
#define END_OF_PACKET                   0x03

namespace SafeEngineering
{
    namespace Comm
    {
        class Serial
        {
        public:
            Serial(asio::io_service& io) : m_serialPort(io)
            {
            }
            
            ~Serial()
            {
            }
            
            void OpenSerial()
            {
                try
                {
                    if(m_serialPort.is_open() == true)
                    {
                        // First. Close COM port because it was opened before
                        CloseSerial();
                    }
                    
                    // Open the COM port
                    m_serialPort.open("/dev/ttyUSB0");  // Name of COM/UART port ("COMx" on Windows or "/dev/ttySx" on Linux)
                    // Set baud rate
                    asio::serial_port_base::baud_rate baud(9600);
                    m_serialPort.set_option(baud);
                    // Set parity
                    asio::serial_port_base::parity parity(asio::serial_port_base::parity::none);
                    m_serialPort.set_option(parity);
                    // Set flow control
                    asio::serial_port_base::flow_control flowCtrl(asio::serial_port_base::flow_control::none);
                    m_serialPort.set_option(flowCtrl);
                    // Set data bits
                    asio::serial_port_base::character_size dataBits(8);
                    m_serialPort.set_option(dataBits);
                    // Set stop bits
                    asio::serial_port_base::stop_bits stopBits(asio::serial_port_base::stop_bits::one);
                    m_serialPort.set_option(stopBits);
                    
                    // Clear data structure
                    memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                    m_packetLen = 0;
                    
                    // Setup asynchonous read operation
                    StartAsyncRead();
                }
                catch(std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }
            
            void CloseSerial()
            {
                m_serialPort.close();
            }
            
            bool SendPacket(const uint8_t* pPacket, size_t len)
            {
                if(len != SERIAL_PACKET_LENGTH)
                    return false;
                if(m_serialPort.is_open() == false)
                    return false;
                    
                if(m_serialPort.write_some(asio::buffer(pPacket, len)) == len)
                {
                    std::cout << "Sent " << len << " bytes" << std::endl;
                    std::cout << Utils::ConvertToHex(pPacket, (int)len) << std::endl;
                    return true;                    
                }
                else
                {
                    return false;
                }
            }
            
        private:
        
            void StartAsyncRead()
            {
                // Clear content of buffer
                m_serialPort.async_read_some(asio::buffer(m_buffer), boost::bind(&Serial::ReadHandler, 
                    this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                std::cout << "Started asynchonous read operation" << std::endl;
            }
            
            void ReadHandler(const asio::error_code& err, std::size_t bytes)
            {
                if(!err)
                {
                    std::cout << "Received " << bytes << " bytes" << std::endl;
                    //for(std::size_t i = 0; i < bytes; i++)
                    //{
                    //    std::cout << Utils::ConvertToHex(m_buffer[i]) << std::endl;
                    //}
                    std::cout << Utils::ConvertToHex(m_buffer, bytes) << std::endl;
                    
                    // Parse serial data and construct new packet
                    for(int i = 0; i < (int)bytes; i++)
                    {
                        // Add new byte into tail of packet
                        if(m_packetLen < SERIAL_PACKET_LENGTH)
                            m_packet[m_packetLen++] = m_buffer[i];
                            
                        // Check if we get whole packet
                        if(m_packetLen >= SERIAL_PACKET_LENGTH)
                        {
                            if(m_packet[0] == (uint8_t)START_OF_PACKET && m_packet[SERIAL_PACKET_LENGTH - 1] == (uint8_t)END_OF_PACKET)
                            {
                                // We got the whole valid packet
                                std::cout << "Send packet to remote device" << std::endl;
                            }
                            else
                            {
                                // Invalid packet
                                std::cout << "Received invalid packet at local device" << std::endl;
                            }
                            // Reset for new packet
                            memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                            m_packetLen = 0;
                        }
                    }
                }
                else
                {
                    std::cout << "Failed: '" << err.message() <<"' when reading data on external device" << std::endl;                    
                }
                
                if(err != asio::error::operation_aborted)
                {
                    // Start next asynchonous read operation
                    StartAsyncRead();
                }
            }
            
        private:
            // Object describes COM port from ASIO library
            asio::serial_port m_serialPort;
            
            // Buffer holds data received from COM port
            uint8_t m_buffer[40];
            
            // Data packet from external device
            uint8_t m_packet[SERIAL_PACKET_LENGTH + 1];
            uint8_t m_packetLen;
            
        };  // Serial class
        
    }   // namespace Comm
} 

#endif // SERIAL_HPP