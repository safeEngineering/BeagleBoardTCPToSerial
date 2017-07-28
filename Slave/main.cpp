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
        // Load settings from json/text file
        SafeEngineering::Utils::Settings appSettings;
        if(SafeEngineering::Utils::LoadSettings(appSettings) == false)
        {
            std::cerr << "Main() function failed to load settings from json/text file" <<std::endl;
            return -1;
        }
        
        // Double-check the ID value was declared for submaster
        if(appSettings.CurrentUnit.ID < 1 || appSettings.CurrentUnit.ID > 8)
        {
            std::cerr << "Invalid ID in settings.json file" <<std::endl;
            return -1;
        }
        
        asio::io_service ios;
        
        SafeEngineering::Comm::Serial serial1(ios);
        // Open UART connection
        serial1.OpenSerial();
        
        // Listen to connections from Master
        SafeEngineering::Comm::Acceptor acceptor1(ios, serial1, appSettings.CurrentUnit.IPAddress, 10001);
        acceptor1.AcceptConnections();
        
        // Listen to connections from Submaster
        SafeEngineering::Comm::Acceptor acceptor2(ios, serial1, appSettings.CurrentUnit.IPAddress, 10002);
        acceptor2.AcceptConnections();
        
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
