#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <asio.hpp>

#include <Serial.hpp>

#include "Utils.hpp"

#include <string>
#include "spdlog/spdlog.h"
#include "DailyFileSink.hpp"

#define SOCKET_BUFFER_LENGTH            100

namespace SafeEngineering
{
    namespace Comm
    {
        class Connection : public boost::enable_shared_from_this<Connection>
        {   
        public:
            typedef boost::shared_ptr<Connection> pointer;
                        
            Connection(asio::io_service& io_service, Serial& serial) : m_socket(io_service), m_delay(io_service), m_serial(serial)
            {
                // The default is server connection
                m_serverSide = true;
                // Socket have not connected yet
                m_connectedSocket = false;
            }
            
            ~Connection()
            {
                std::cout << "Destroy TCP/IP connection" << std::endl;
            }

            // The event will signal whenever connection was dropped
            boost::signals2::signal<void(Connection::pointer)> m_ConnectionDropped;

            // Get underlying TCP/IP socket
            asio::ip::tcp::socket& Socket()
            {
                return m_socket;
            }
            
            // Register data from UART port
            void RegisterSerialData()
            {
                // Register signal from serial object
                m_serial.m_DataReceived.connect(boost::bind(&Connection::HandleSerialData, shared_from_this(), _1, _2));
            }
            
            // Start reading data from remote client
            void Start()
            {                
                // Keep remote endpoint for logging
                m_remoteEndpoint = m_socket.remote_endpoint();
                // Socket have already connected
                m_connectedSocket = true;
                // And then, we wait for TCP/IP data
                std::cout << "Waiting for data from endpoint:" << m_remoteEndpoint << std::endl;                
                m_socket.async_read_some(asio::buffer(m_buffer, SOCKET_BUFFER_LENGTH),
                    boost::bind(&Connection::HandleRead, shared_from_this(),
                    asio::placeholders::error,
                    asio::placeholders::bytes_transferred));
            }
            
            // Connect to remote server
            void Connect(const std::string& serverIP, int serverPort)
            { 
                // Mark this connection as client connection
                m_serverSide = false;
                
                // Save connection parameters for later use
                m_serverIP = serverIP;
                m_serverPort = serverPort;
                
                // Register serial Data
                RegisterSerialData();
                    
                asio::ip::tcp::endpoint ep(asio::ip::address::from_string(serverIP), serverPort);
                std::cout << "Trying to connect to " << ep << std::endl;
                m_socket.async_connect(ep, boost::bind(&Connection::HandleConnect, shared_from_this(), asio::placeholders::error));
            }
            
            // Close the TCP/IP connection
            void Close()
            {
                if(m_socket.is_open())
                    m_socket.close();
            }
            
        private:
            // Reconnect to the remote server
            void Reconnect()
            {
                asio::ip::tcp::endpoint ep(asio::ip::address::from_string(m_serverIP), m_serverPort);
                std::cout << "Trying to reconnect to " << ep << std::endl;
	            
	            spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << "Trying to reconnect to " << ep;

                m_socket.async_connect(ep, boost::bind(&Connection::HandleConnect, shared_from_this(), asio::placeholders::error));
            }
            
            void HandleRead(const asio::error_code& err, const std::size_t& bytes_received)
            {
                if(!err)
                {
                    std::cout << "Received " << bytes_received << " bytes from remote endpoint:" << m_remoteEndpoint << std::endl;
                    std::cout << Utils::ConvertToHex(m_buffer, bytes_received) << std::endl;
	                
	                /***************************************************/                    
	                //AE Modify Data for return echo for testing.
	                if (bytes_received > 5)
	                {
		            //    m_buffer[5] = 0x48;    
	                }
                    /***************************************************/                    
                    
                    // Send data to UART port
                    std::cout << "Forward packet to external UART device" << std::endl;
                    m_serial.SendPacket(m_buffer, bytes_received);
                    
                    // Read next data
                    m_socket.async_read_some(asio::buffer(m_buffer, SOCKET_BUFFER_LENGTH),
                        boost::bind(&Connection::HandleRead, shared_from_this(),
                        asio::placeholders::error,
                        asio::placeholders::bytes_transferred));
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' in HandleRead on endpoint:" << m_remoteEndpoint << std::endl;
                    ProcessBrokenConnection();
                }
            }
            
            void HandleWrite(const asio::error_code& err, const std::size_t& bytes_sent)
            {
                if(!err)
                {
                    std::cout << "Sent " << bytes_sent << " bytes to endpoint:" << m_remoteEndpoint << std::endl;
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' in HandleWrite on endpoint:" << m_remoteEndpoint << std::endl;
                    ProcessBrokenConnection();
                }
            }
            
            void HandleConnect(const asio::error_code& err)
            {
                if(!err)
                {
                    // Initialize data reading for new connection
                    Start();
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' in HandleConnect" << std::endl;
					
	                spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << "Failed: '" << err.message() << "' in HandleConnect";

	                // Delay asynchronously 1s after that we will try reconnecing to the server
                    m_delay.expires_from_now(boost::posix_time::seconds(1));
                    m_delay.async_wait(boost::bind(&Connection::Reconnect, shared_from_this()));
                }
            }
            
            void HandleSerialData(uint8_t* pPacket, uint8_t len)
            {
                if(m_connectedSocket == false)
                {
                    std::cerr << "TCP/IP connection have not established yet" << std::endl;
                    return;
                }
                    
                std::cout << "Forward packet to remote device throughout TCP/IP" << std::endl;
                // Copy data from serial's buffer to local tx buffer
                memcpy(m_txbuffer, pPacket, len);
                m_txbufferLen = len;
                
                // Write data to TCP/IP network
                asio::async_write(m_socket, asio::buffer(m_txbuffer, m_txbufferLen), 
                    boost::bind(&Connection::HandleWrite, shared_from_this(),
                    asio::placeholders::error,
                    asio::placeholders::bytes_transferred));
            }
            
            void ProcessBrokenConnection()
            {
                // Close underlying socket
                m_socket.close();
                // Socket have already broken
                m_connectedSocket = false;
                if(m_serverSide == true)
                {
                    // Signal to acceptor that the connection was dropped
                    m_ConnectionDropped(shared_from_this());
                    m_serial.m_DataReceived.disconnect(boost::bind(&Connection::HandleSerialData, shared_from_this(), _1, _2));
                }
                else
                {	                
	                spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << "Trying to reconnect to ";

                    // Delay asynchronously 1s after that we will try reconnecing to the server
                    m_delay.expires_from_now(boost::posix_time::seconds(1));
                    m_delay.async_wait(boost::bind(&Connection::Reconnect, shared_from_this()));
                }
            }
            
        private:
            // TCP/IP socket object
            asio::ip::tcp::socket m_socket;
            // Delay object
            asio::deadline_timer m_delay;
            
            // UART/COM serial object
            Serial& m_serial;
            // Buffer holds data received remote endpoint
            uint8_t m_buffer[SOCKET_BUFFER_LENGTH + 1];
            // Buffer holds data will be sent to remote endpoint
            uint8_t m_txbuffer[SOCKET_BUFFER_LENGTH + 1];
            uint8_t m_txbufferLen;
            
            // Remote endpoint of the connection
            asio::ip::tcp::endpoint m_remoteEndpoint;
            // Server connection parameters (it used to reconnect to server)
            std::string m_serverIP;
            int m_serverPort;
            
            // The flag specifies this connection is client side at or server side
            bool m_serverSide;
            // The flag specifies whether socket is actually connected or not
            bool m_connectedSocket;
                        
        };  // Connection class
    }   
    
}   // namespace SafeEngineering

#endif // TCP_CONNECTION_HPP
