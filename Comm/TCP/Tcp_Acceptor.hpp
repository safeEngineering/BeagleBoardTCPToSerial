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
            Acceptor(asio::io_service& io_service, Serial& serial, const std::string& host, int port, std::string clientIP) : m_ios(io_service), m_serial(serial),
                m_acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port)), m_clientIP(clientIP)
            {
                m_totalConnections = 0;
                m_currentConnection = nullptr;
            }
            
            bool AcceptConnections()
            {
                try
                {   
                    SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<Connection>(new Connection(m_ios, m_serial));
                    // Register Dropped event
                    new_connection->m_ConnectionDropped.connect(boost::bind(&Acceptor::HandleDroppedConnection, this, _1));
   
                    std::cout << "Waiting for new client" << std::endl;
                    m_acceptor.async_accept(new_connection->Socket(),
                        boost::bind(&Acceptor::HandleAccept, this, new_connection, asio::placeholders::error));
                }
                catch(std::exception& e)
                {
                    std::cerr << "AcceptConnection thrown exception: '" << e.what() << "'" << std::endl;
                    return false;
                }
                return true;
            }
        
        private:
            void HandleAccept(Connection::pointer new_connection, const asio::error_code& err)
            {
                std::string remoteIP;
                
                if(!err)
                {
                    remoteIP = new_connection->Socket().remote_endpoint().address().to_string();
                    if(remoteIP != m_clientIP)
                    {
                        std::cout << "Close connection from invalid IP: " << remoteIP << std::endl;
                        new_connection->m_ConnectionDropped.disconnect(boost::bind(&Acceptor::HandleDroppedConnection, this, _1));
                        new_connection->Close();
                    }
                    else
                    {
                        ///////////////////////////////////////////////////////////////////////////////////////////////////////
                        // Now, we no need to guard m_totalConnections variable because all opeartions were in the same thread
                        // However, the synchronous was needed whenever we call asio::io_service::run from more than threads
                        ///////////////////////////////////////////////////////////////////////////////////////////////////////
                        if(m_totalConnections > 0 && m_currentConnection != nullptr)    // Request to close the previous connection
                        {
                            std::cout << "Close the previous connection" << std::endl;
                            m_currentConnection->Close();
                        }
                        
                        // Increase the total connections
                        m_totalConnections++;
                        // Assign this connection as currently active connection
                        m_currentConnection = new_connection;
                        std::cout << "Accepted request from endpoint:" << new_connection->Socket().remote_endpoint() << std::endl;
                        // Register Data event from Serial class
                        new_connection->RegisterSerialData();
                        // Initialize data reading for new connection
                        new_connection->Start();
                    }
                    
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
            
            void HandleDroppedConnection(Connection::pointer dropped_connection)
            {
                /////////////////////////////////////////////////////////////////////////////////////////////////////
                // Now, we no need to guard m_totalConnections variable because all opeartions were in the same thread
                // However, the synchronous was needed whenever we call asio::io_service::run from more than threads
                /////////////////////////////////////////////////////////////////////////////////////////////////////
                // Decrease the total connections
                m_totalConnections--;
                // Remove callback function
                dropped_connection->m_ConnectionDropped.disconnect(boost::bind(&Acceptor::HandleDroppedConnection, this));
            }
            
        private:
            // TCP/IP socket object
            asio::io_service& m_ios;
            // UART/COM serial object
            Serial& m_serial;
            // TCP/IP connection acceptance object
            asio::ip::tcp::acceptor m_acceptor;
            // The total connections
            int m_totalConnections;
            // Current TCP/IP connection
            SafeEngineering::Comm::Connection::pointer m_currentConnection;
            // Valid IP
            std::string m_clientIP;
            
        };  // Acceptor class
        
    }   // namespace Comm
}

#endif // TCP_ACCEPTOR_HPP