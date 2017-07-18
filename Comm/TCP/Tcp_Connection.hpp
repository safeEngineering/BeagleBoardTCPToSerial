#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include <Serial.hpp>

#define SOCKET_BUFFER_LENGTH            100

namespace SafeEngineering
{
    namespace Comm
    {
        class Connection : public boost::enable_shared_from_this<Connection>
        {   
        public:
            typedef boost::shared_ptr<Connection> pointer;
            
            //Connection(asio::io_service& io_service) : m_socket(io_service), m_COM(io_service)
            Connection(asio::io_service& io_service, Serial& serial) : m_socket(io_service), m_serial(serial)
            {
            
            }
            
            // Get underlying TCP/IP socket
            asio::ip::tcp::socket& Socket()
            {
                return m_socket;
            }
                        
            // Start reading data from remote client
            void Start()
            {                
                // First, register signal from serial object
                m_serial.m_DataReceived.connect(boost::bind(&Connection::HandleSerialData, shared_from_this(), _1, _2));
                
                // Keep remote endpoint for logging
                m_remoteEndpoint = m_socket.remote_endpoint();
                
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
                asio::ip::tcp::endpoint ep(asio::ip::address::from_string(serverIP), serverPort);
                std::cout << "Trying to connect to " << ep << std::endl;
                m_socket.async_connect(ep, boost::bind(&Connection::HandleConnect, shared_from_this(), asio::placeholders::error));
            }
            
            // Close the TCP/IP connection
            void Close()
            {
                m_socket.close();
            }
            
        private:
            void HandleRead(const asio::error_code& err, std::size_t bytes_received)
            {
                if(!err)
                {
                    std::cout << "Received " << bytes_received << " bytes from remote endpoint:" << m_remoteEndpoint << std::endl;
                    std::cout << Utils::ConvertToHex(m_buffer, bytes_received) << std::endl;
                    
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
                    std::cout << "HandleRead closes the connection with endpoint:" << m_remoteEndpoint << std::endl;
                    m_socket.close();
                }
            }
            
            void HandleWrite(const asio::error_code& err, std::size_t bytes_sent)
            {
                if(!err)
                {
                    std::cout << "Sent " << bytes_sent << " bytes to endpoint:" << m_remoteEndpoint << std::endl;
                }
                else
                {
                    std::cout << "HandleWrite closes the connection with endpoint:" << m_remoteEndpoint << std::endl;
                    m_socket.close();
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
                    std::cerr << "Failed: '" << err.message() <<"' when connecting to server" << std::endl;
                }
            }
            
            void HandleSerialData(uint8_t* pPacket, uint8_t len)
            {
                // Copy data from serial's buffer to local tx buffer
                memcpy(m_txbuffer, pPacket, len);
                m_txbufferLen = len;
                
                // Write data to TCP/IP network
                asio::async_write(m_socket, asio::buffer(m_txbuffer, m_txbufferLen), 
                    boost::bind(&Connection::HandleWrite, shared_from_this(),
                    asio::placeholders::error,
                    asio::placeholders::bytes_transferred));
            }
            
        private:
            // TCP/IP socket object
            asio::ip::tcp::socket m_socket;
            // UART/COM serial object
            Serial& m_serial;
            // Buffer holds data received remote endpoint
            uint8_t m_buffer[SOCKET_BUFFER_LENGTH + 1];
            // Buffer holds data will be sent to remote endpoint
            uint8_t m_txbuffer[SOCKET_BUFFER_LENGTH + 1];
            uint8_t m_txbufferLen;
            
            // Remote endpoint of the connection
            asio::ip::tcp::endpoint m_remoteEndpoint;
            
        };  // Connection class
    }   
    
}   // namespace SafeEngineering

#endif // TCP_CONNECTION_HPP
