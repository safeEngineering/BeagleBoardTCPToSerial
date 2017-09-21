#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>


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

void InitialiseLogFiles()
{
	try
	{
		
		std::string strLogPath = "/var/www/html/";
		if (mkpath(strLogPath, 0744))
		{
			std::cerr << "Unable to create log path: " << strLogPath << std::endl;
			return;
		}
		
		spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::milliseconds(5000));
		
		
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPath + "log", "txt", 8 * 1024 /* * 1024 */, 3, false));
		auto log = spdlog::create("log", begin(sinks), end(sinks));
		log->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%l] %v");
		// FIXME: level should be err normally
		log->set_level(spdlog::level::info);

		std::vector<spdlog::sink_ptr> test_sinks;
		test_sinks.push_back(std::make_shared<SafeEngineering::DailyFileSink_st>(strLogPath, "test_log", "txt", 40, false));
		auto test_log = spdlog::create("test_log", begin(test_sinks), end(test_sinks));
		test_log->set_pattern("%v");
		
		spdlog::get("test_log")->info() << SafeEngineering::Utils::timeString(std::chrono::system_clock::now()) << " Starting";
		//log->error() << "Starting E...";
		log->info()  << "Starting I...";

		//log->info()  << "Ending I...";
		//spdlog::get("test_log")->info() << timeString(std::chrono::system_clock::now()) << " Ending";
		
		// Release and close all loggers
		log->flush();
		test_log->flush();
		
		//No need to close logs as the process runs forever
		//spd::drop_all();
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
	    struct ifaddrs *ifap, *ifa;
	    struct sockaddr_in *sa;
	    char *addr;

	    getifaddrs(&ifap);
	    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		    if (ifa->ifa_addr->sa_family == AF_INET) {
			    sa = (struct sockaddr_in *) ifa->ifa_addr;
			    addr = inet_ntoa(sa->sin_addr);
			    printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
		    }
	    }

	    freeifaddrs(ifap);
	    
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
        
        SafeEngineering::Comm::Serial serial1(ios);
        // Open UART connection
        serial1.OpenSerial();
        
        // Listen to connections from Master
        SafeEngineering::Comm::Acceptor acceptor1(ios, serial1, appSettings.CurrentUnit.IPAddress, 10001, appSettings.Units[0].IPAddress);
        acceptor1.AcceptConnections();
        
	    std::cout << "Master IP:" << appSettings.Units[0].IPAddress << std::endl;
	    std::cout << "SubMaster IP:" << appSettings.Units[8].IPAddress << std::endl;
	    std::cout << "My IP:" << appSettings.CurrentUnit.IPAddress << std::endl;
	    
        // Listen to connections from Submaster
        SafeEngineering::Comm::Acceptor acceptor2(ios, serial1, appSettings.CurrentUnit.IPAddress, 10002, appSettings.Units[8].IPAddress);  //AE [8] was [9]
        acceptor2.AcceptConnections();
	    
	    InitialiseLogFiles();
        
        // Run the ASIO service
        ios.run();
        
        // Close UART connection
        serial1.CloseSerial();
    }
    catch(std::exception& e)
    {
        std::cerr << "Main thrown exception: '" << e.what() << "'" << std::endl;
    }
    
	return 0;
}
