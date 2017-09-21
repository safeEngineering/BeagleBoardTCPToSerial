#include <iostream>
#include <ctime>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include "Serial.hpp"
#include "Tcp_Connection.hpp"
#include "Tcp_Acceptor.hpp"

#include "Utils.hpp"
#include <string>
#include "spdlog/spdlog.h"
#include "DailyFileSink.hpp"

namespace spd = spdlog;

const char PATH_SEP = '/';

int mkpath(const std::string &strPath, unsigned int mode)
{
	std::size_t startIndex = 0;
	std::size_t endIndex = 0;
	std::string currentPath;
	currentPath.reserve(strPath.size());
	struct stat buf;
	while (startIndex < strPath.size())
	{
		endIndex = strPath.find(PATH_SEP, startIndex);
		std::size_t length;
		if (endIndex == 0)
		{
		    // root
			currentPath += PATH_SEP;
			startIndex++;
			continue;
		}
		else
		{
			if (endIndex == std::string::npos)
			{
			    // last dir
				endIndex = strPath.size();
			}
			length = endIndex - startIndex;

			if (length == 0)
			{
				return -1;
			}

			std::string dir = strPath.substr(startIndex, length);
			currentPath += dir;

			if (stat(currentPath.c_str(), &buf) != 0)
			{
				if (mkdir(currentPath.c_str(), mode) != 0 && errno != EEXIST)
				{
					return -1;
				}
			}
			else if (!S_ISDIR(buf.st_mode))
			{
				return -1;
			}
		}

		currentPath += PATH_SEP;
		startIndex = endIndex + 1;
	}
	return 0;
}

void InitialiseLogFiles(std::string siteName, SafeEngineering::Utils::UnitType unitType)
{
	try
	{
		
		std::string strLogPath = "/home/debian/web-app/public/log/";
		if (mkpath(strLogPath, 0744))
		{
			std::cerr << "Unable to create log path: " << strLogPath << std::endl;
			return;
		}
		
		spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::milliseconds(5000));
		
		
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPath + "E23CommsDataLog-" + siteName, "txt", 32 * 1024 , 3, false));
		auto datalog = spdlog::create("E23DataLog", begin(sinks), end(sinks));
		datalog->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%l] %v");
		// FIXME: level should be err normally
		datalog->set_level(spdlog::level::info);

		std::vector<spdlog::sink_ptr> test_sinks;
		test_sinks.push_back(std::make_shared<SafeEngineering::DailyFileSink_st>(strLogPath, "E23CommsStatusLog-" + siteName, "txt", 30, false));
		auto statusLog = spdlog::create("E23StatusLog", begin(test_sinks), end(test_sinks));
		statusLog->set_pattern("%v");
		
		if (unitType == SafeEngineering::Utils::UnitType::MASTER)	    
		{
			spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting E23 Status Log (MASTER)";
		}
		else if (unitType == SafeEngineering::Utils::UnitType::SUBMASTER)	    
		{
			spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting E23 Status Log (SUBMASTER)";
		}
		if (unitType == SafeEngineering::Utils::UnitType::SLAVE)	    
		{
			spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting E23 Status Log (SLAVE)";
		}
		else
		{
			spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting E23 Status Log (UNKNOWN TYPE ERROR)";
		}
		
		datalog->info()  << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting E23 Data Log";

		datalog->flush();
		statusLog->flush();
		
	}
	// Exceptions will only be thrown upon failed logger or sink construction (not during logging)
	catch (const spd::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}


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
        
        // Double-check the ID value was declared for master, submaster or slave
	    if ((appSettings.CurrentUnit.ID < 0) || (appSettings.CurrentUnit.ID > 9))
        {
            std::cerr << "Invalid ID in settings.json file" << std::endl;
            return -1;
        }
        
        // Check number of elements in units array
        if((int)appSettings.Units.size() != MAX_REMOTE_UNIT_NUMBERS)
        {
            std::cerr << "Number of units in settings.json file was invalid" << std::endl;
            return -1;  
        }
        
        asio::io_service ios;
        
        // Construct serial object
        SafeEngineering::Comm::Serial serial1(ios);
        // Open UART connection
        serial1.OpenSerial();
        
	    if (appSettings.CurrentUnit.Type == SafeEngineering::Utils::UnitType::MASTER)
	    {
		    std::cout << "STARTING AS MASTER!!!" << std::endl;
		    
		    std::cout << "Master IP:" << appSettings.Units[0].IPAddress << std::endl;
		    std::cout << "SubMaster IP:" << appSettings.Units[8].IPAddress << std::endl;
		    std::cout << "My IP:" << appSettings.CurrentUnit.IPAddress << std::endl;
	        	    
        // First, make a connection to submaster
		    for (int i = 0; i < (int)appSettings.Units.size(); i++)
		    {
			    if (appSettings.Units[i].Type == SafeEngineering::Utils::UnitType::SUBMASTER)
			    {
			        // Construct TCP/IP object
				    SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<SafeEngineering::Comm::Connection>(new SafeEngineering::Comm::Connection(ios, serial1));                
				    new_connection->Connect(appSettings.Units[i].IPAddress, 10001);
			    }
		    }

		            // Second, make connections to slaves
		    for (int i = 0; i < (int)appSettings.Units.size(); i++)
		    {			    
			    if (appSettings.Units[i].Type == SafeEngineering::Utils::UnitType::SLAVE)
			    {
			        // Construct TCP/IP object
				    SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<SafeEngineering::Comm::Connection>(new SafeEngineering::Comm::Connection(ios, serial1));
				    new_connection->Connect(appSettings.Units[i].IPAddress, 10001);
				    //break;  // Now, we connect to one-only-one slave module
			    }
		    }
	    }
	    else if (appSettings.CurrentUnit.Type == SafeEngineering::Utils::UnitType::SUBMASTER)	    
	    {
		    std::cout << "STARTING AS SUBMASTER!!!" << std::endl;
		    
		    std::cout << "Master IP:" << appSettings.Units[0].IPAddress << std::endl;
		    std::cout << "SubMaster IP:" << appSettings.Units[8].IPAddress << std::endl;
		    std::cout << "My IP:" << appSettings.CurrentUnit.IPAddress << std::endl;
	    		    
		    // First, listen to connections from master
		    SafeEngineering::Comm::Acceptor acceptor(ios, serial1, appSettings.CurrentUnit.IPAddress, 10001, appSettings.Units[0].IPAddress);
		    acceptor.AcceptConnections();
        
		    // Second, make connections to slaves
		    for (int i = 0; i < (int)appSettings.Units.size(); i++)
		    {
			    if (appSettings.Units[i].Type == SafeEngineering::Utils::UnitType::SLAVE)
			    {
			        // Construct TCP/IP object
				    SafeEngineering::Comm::Connection::pointer new_connection = boost::shared_ptr<SafeEngineering::Comm::Connection>(new SafeEngineering::Comm::Connection(ios, serial1));
				    new_connection->Connect(appSettings.Units[i].IPAddress, 10002);                
				    //break;  // Now, we connect to one-only-one slave module
			    }
		    }
	    }
	    else if (appSettings.CurrentUnit.Type == SafeEngineering::Utils::UnitType::SLAVE)	    
	    {
		    std::cout << "STARTING AS SLAVE!!!" << std::endl;
		    		    
		    std::cout << "Master IP:" << appSettings.Units[0].IPAddress << std::endl;
		    std::cout << "SubMaster IP:" << appSettings.Units[8].IPAddress << std::endl;
		    std::cout << "My IP:" << appSettings.CurrentUnit.IPAddress << std::endl;
	    
			// Listen to connections from Master
		    SafeEngineering::Comm::Acceptor acceptor1(ios, serial1, appSettings.CurrentUnit.IPAddress, 10001, appSettings.Units[0].IPAddress);
		    acceptor1.AcceptConnections();
        		    
			// Listen to connections from Submaster
		    SafeEngineering::Comm::Acceptor acceptor2(ios, serial1, appSettings.CurrentUnit.IPAddress, 10002, appSettings.Units[8].IPAddress);  //AE [8] was [9]
		    acceptor2.AcceptConnections();
	        
	    }
	    else 
	    {
		    std::cerr << "UNKNOWN START UP MOE VALID OPTIONS ARE MASTER, SUBMASTER and SLAVE" << std::endl;
		    serial1.CloseSerial();
		    return -1;  		    
	    }
				
	    InitialiseLogFiles(appSettings.SiteName, appSettings.CurrentUnit.Type);
        	    
	    
        // Run the ASIO service
	    try
	    {	    	    
		    ios.run();
	    }
	    catch (const std::exception& ex)
	    {
		    std::cerr << "Controller stopped with an exception: " << ex.what();		    
	    }
	    catch (...)
	    {
		    std::cerr << "Controller stopped with unexpected exception";		    
	    }
	    
	    spdlog::get("E23DataLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Ending";		
	    spdlog::get("E23StatusLog")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Ending";
		
		// Release and close all loggers
	    spdlog::get("E23DataLog")->flush();
	    spdlog::get("E23StatusLog")->flush();
		
		spdlog::drop_all();
	    	    
        // Close UART connection
        serial1.CloseSerial();
	     
	    
	    std::cout << "Finished";
    }
    catch(std::exception& e)
    {
        std::cerr << "Main thrown exception: '" << e.what() << "'" << std::endl;
    }
    
	return 0;
}
