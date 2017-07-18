#include <stdio.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include "Serial.hpp"
#include "Tcp_Connection.hpp"
#include "Tcp_Acceptor.hpp"

int main(int argc, char **argv)
{
    try
    {
        asio::io_service ios;
        
        SafeEngineering::Comm::Serial serial1(ios);
        // Open UART connection
        serial1.OpenSerial();
        
        // Listen to new connections
        //SafeEngineering::Comm::Acceptor acceptor(ios, serial1, "192.168.1.103", 8000);
        SafeEngineering::Comm::Acceptor acceptor(ios, serial1, "10.10.1.144", 8000);
        acceptor.AcceptConnections();
        
        // Run the ASIO service
        ios.run();
        
        // Close UART connection
        serial1.CloseSerial();
    }
    catch(std::exception& e)
    {
        std::cerr << "Main thrown exception: '" << e.what() << "'" <<std::endl;
    }
    
	return 0;
}
