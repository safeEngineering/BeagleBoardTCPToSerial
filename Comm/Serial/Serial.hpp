#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <iostream>
#include <iomanip>
#include <deque>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <asio.hpp>

#define SERIAL_BUFFER_LENGTH            40
#define SERIAL_PACKET_LENGTH            8

#define START_OF_PACKET                 0x02
#define END_OF_PACKET                   0x03

#include "SerialPacket.hpp"
#include "Utils.hpp"

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
            
            // The event will signal whenever receiving the data packet from external serial device
            boost::signals2::signal<void(uint8_t*, uint8_t)> m_DataReceived;
            
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
                    m_serialPort.open("/dev/ttyS1");  // Name of COM/UART port ("COMx" on Windows or "/dev/ttySx" on Linux)
                    // Set baud rate
                    asio::serial_port_base::baud_rate baud(38400);
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
                
                // Construct new packet for the queue
                SerialPacket packet(pPacket, len);
                
                // Keep in mind. Calling to empty() will been ensured as atomic operation!!!
                bool write_in_progress = !m_writePackets.empty();                
                m_writePackets.push_back(packet);
                if(!write_in_progress)
                {
                    std::cout << "Sending bytes: " << Utils::ConvertToHex(pPacket, (int)len) << " to UART port" << std::endl;
                    
                    // Send this packet to serial port now
                    asio::async_write(m_serialPort, asio::buffer(m_writePackets.front().Data(), m_writePackets.front().Length()), 
                        boost::bind(&Serial::HandleWrite, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                }
                else
                {
                    std::cout << "Push the packet to the queue" << std::endl;
                }
                
                return true;
            }
            
        private:        
            void StartAsyncRead()
            {
                // Clear content of buffer
                m_serialPort.async_read_some(asio::buffer(m_buffer), boost::bind(&Serial::ReadHandler, 
                    this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                std::cout << "Started asynchonous UART read operation" << std::endl;
            }
            
            void ReadHandler(const asio::error_code& err, std::size_t bytes)
            {
                if(!err)
                {
                    std::cout << "Received " << bytes << " bytes from UART" << std::endl;
                    std::cout << Utils::ConvertToHex(m_buffer, bytes) << std::endl;
                    
                    // Parse serial data and construct new packet
                    for(int i = 0; i < (int)bytes; i++)
                    {
                        if(m_buffer[i] == (uint8_t)START_OF_PACKET)
                        {
                            // Reset for new packet
                            memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                            m_packetLen = 0;
                        }

                        // Add new byte into tail of packet
                        if(m_packetLen < SERIAL_PACKET_LENGTH)
                            m_packet[m_packetLen++] = m_buffer[i];
                            
                        // Check if we get whole packet
                        if(m_packetLen >= SERIAL_PACKET_LENGTH)
                        {
                            if(m_packet[0] == (uint8_t)START_OF_PACKET && m_packet[SERIAL_PACKET_LENGTH - 1] == (uint8_t)END_OF_PACKET)
                            {
                                // We got the whole valid packet                                
                                m_DataReceived(m_packet, m_packetLen);
                            }
                            else
                            {
                                // Invalid packet
                                std::cout << "Received invalid packet from external device" << std::endl;
                                //std::cout << Utils::ConvertToHex(m_packet, m_packetLen) << std::endl;
                            }
                            // Reset for new packet
                            memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                            m_packetLen = 0;
                        }
                    }
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' when reading data from external device" << std::endl;
                }
                
                if(err != asio::error::operation_aborted)
                {
                    // Start next asynchonous read operation
                    StartAsyncRead();
                }
            }
            
            void HandleWrite(const asio::error_code& err, std::size_t bytes)
            {
                if(!err)
                {
                    std::cout << bytes <<  " bytes were sent successfully" << std::endl;
                    
                    // Remote processed packet from queue
                    m_writePackets.pop_front();
                    if(!m_writePackets.empty())
                    {
                        std::cout << "Sending bytes: " << Utils::ConvertToHex(m_writePackets.front().Data(), m_writePackets.front().Length()) << " to UART port" << std::endl;
                        
                        // Send queued packet to serial port now
                        asio::async_write(m_serialPort, asio::buffer(m_writePackets.front().Data(), m_writePackets.front().Length()), 
                            boost::bind(&Serial::HandleWrite, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                    }
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' when writing data to external device" << std::endl;
                }
            }
            
        private:
            // Object describes COM port from ASIO library
            asio::serial_port m_serialPort;
            
            // Buffer holds data received from COM port
            uint8_t m_buffer[SERIAL_BUFFER_LENGTH + 1];
            
            // Data packet from external device
            uint8_t m_packet[SERIAL_PACKET_LENGTH + 1];
            uint8_t m_packetLen;
            
            // The queue holds packets needed writting to hardware serial port
            std::deque<SerialPacket> m_writePackets;
                            
        };  // Serial class
        
    }   // namespace Comm
} 

#endif // SERIAL_HPP
