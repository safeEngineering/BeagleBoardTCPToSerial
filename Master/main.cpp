#include <iostream>
#include <ctime>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include "Serial.hpp"
#include "Tcp_Connection.hpp"

int main(int argc, char **argv)
{
    try
    {
        SafeEngineering::Utils::Settings appSettings;
        if(SafeEngineering::Utils::LoadSettings(appSettings) == false)
        {
            std::cerr << "Main() function failed to load settings from json/text file" <<std::endl;
            return -1;            
        }
        
        // Double-check the ID value was declared for master
        if(appSettings.CurrentUnit.ID != 0)
        {
            std::cerr << "Invalid ID in settings.json file" <<std::endl;
            return -1;
        }
        
        asio::io_service ios;
        
        // Construct serial object
        SafeEngineering::Comm::Serial serial1(ios);
        // Open UART connection
        serial1.OpenSerial();
        
        // First, make a connection to submaster
        for(int i = 0; i < (int)appSettings.Units.size(); i++)
        {
            if(appSettings.Units[i].Type == SafeEngineering::Utils::UnitType::SUBMASTER)
            {
                // Construct TCP/IP object
                SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<SafeEngineering::Comm::Connection>(new SafeEngineering::Comm::Connection(ios, serial1));
                new_connection->Connect(appSettings.Units[i].IPAddress, 10001);
            }
        }
                
        // Second, make connections to slaves
        for(int i = 0; i < (int)appSettings.Units.size(); i++)
        {
            if(appSettings.Units[i].Type == SafeEngineering::Utils::UnitType::SLAVE)
            {
                // Construct TCP/IP object
                SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<SafeEngineering::Comm::Connection>(new SafeEngineering::Comm::Connection(ios, serial1));
                new_connection->Connect(appSettings.Units[i].IPAddress, 10001);
                //break;  // Now, we connect to one-only-one slave module
            }
        }
        
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
