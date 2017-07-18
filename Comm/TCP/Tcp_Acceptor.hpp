#ifndef TCP_ACCEPTOR_HPP
#define TCP_ACCEPTOR_HPP

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include "Tcp_Connection.hpp"
#include <Serial.hpp>

namespace SafeEngineering
{
    namespace Comm
    {
        class Acceptor
        {
        public:
            Acceptor(asio::io_service& io_service, Serial& serial, const std::string& host, int port) : m_ios(io_service), m_serial(serial),
                m_acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port))
            {
                
            }
            
            bool AcceptConnections()
            {
                try
                {
                    SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<Connection>(new Connection(m_ios, m_serial));
                    
                    std::cout << "Waiting for new client" << std::endl;
                    m_acceptor.async_accept(new_connection->Socket(),
                        boost::bind(&Acceptor::HandleAccept, this, new_connection, asio::placeholders::error));
                }
                catch(std::exception& e)
                {
                    std::cerr << "AcceptConnection thrown exception: '" << e.what() << "'" <<std::endl;
                    return false;
                }
                return true;
            }
        
        private:
            void HandleAccept(Connection::pointer new_connection, const asio::error_code& err)
            {
                if(!err)
                {
                    std::cout << "Accepted request from endpoint:" << new_connection->Socket().remote_endpoint() << std::endl;
                    // Initialize data reading for new connection
                    new_connection->Start();
                    
                    // Listen to new request
                    if(!AcceptConnections())
                    {
                        std::cerr << "Failure during call to accept" << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() <<"' when accepting new connection" << std::endl;
                }
            }
            
        private:
            // TCP/IP socket object
            asio::io_service& m_ios;
            // UART/COM serial object
            Serial& m_serial;
            // TCP/IP connection acceptance object
            asio::ip::tcp::acceptor m_acceptor;            
            
        };  // Acceptor class
        
    }   // namespace Comm
}

#endif // TCP_ACCEPTOR_HPP