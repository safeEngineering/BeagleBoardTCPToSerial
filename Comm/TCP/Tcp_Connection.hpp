#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include <Serial.hpp>

namespace SafeEngineering
{
    namespace Comm
    {
        class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
        {   
        public:
            typedef boost::shared_ptr<tcp_connection> pointer;
            
            tcp_connection(asio::io_service& io_service) : m_socket(io_service), m_COM(io_service)
            {
                
            }
            
            // Get underlying TCP/IP socket
            asio::ip::tcp::socket& Socket()
            {
                return m_socket;
            }
            
            // Get underlying UART/COM
            Serial& COM()
            {
                return m_COM;
            }
            
            // Create new TCP/IP connection for cient's request
            void Start()
            {
                
            }
            
            void Connect(const std::string& serverIP, int serverPort)
            {
                
            }
            
        private:
            void HandleRead(const asio::error_code& error, std::size_t bytes_received)
            {
                
            }
            
            void HandleWrite(const asio::error_code& error, std::size_t bytes_sent)
            {
                
            }
            
        private:
            // TCP/IP socket object
            asio::ip::tcp::socket m_socket;
            
            // UART/COM serial object
            Serial m_COM;
            
        };  // tcp_connection class
    }   
    
}   // namespace SafeEngineering

#endif // TCP_CONNECTION_HPP
