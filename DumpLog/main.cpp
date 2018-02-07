/************************************************************
 * main.cpp
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		6 Oct 2015	1.0.0    Original
 ************************************************************/

#include "Controller.h"
#include "FileUtil.h"
#include "DailyFileSink.h"
#include "spdlog\spdlog.h"
#include "Utils.hpp"
#include "NTP.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <fstream>
#include <json.hpp>


void Init()
{
	
	nlohmann::json settingsjson =
	{						
		{ "IP_Configuration", "10.0.0.100" },
		{ "Mask_Configuration", "255.255.255.0" },
		{ "Gateway_Configuration", "10.0.0.138" },
		{ "NTP_Configuration", "216.239.35.4" },
		{ "DNS_Configuration", "8.8.8.8" }
	};
		
	std::ofstream outfile("QRFL_Network_ConfigurationB.json");
	outfile << settingsjson;
	
	nlohmann::json settingsjsonA =
	{						
		{ "IP", "10.0.0.100" },
		{ "Mask", "255.255.255.0" },
		{ "Gateway", "10.0.0.138" },
		{ "NTP_IP", "216.239.35.4" },
		{ "DNS", "8.8.8.8" }
	};
		
	std::ofstream outfileA("QRFL_Network_SettingsB.json");
	outfileA << settingsjsonA;
}

void Read()
{
	// Load json file from current directory
	std::ifstream jsonFile("QRFL_Network_ConfigurationB.json");	
	// JSON object
	nlohmann::json settingsjson;
	// Deserialize json object from text content
		
	jsonFile >> settingsjson;
	
		
	auto value = settingsjson.find("Gateway_Configuration");
	if (value != settingsjson.end()) 
	{				
		std::string ipaddress_str = value->get<std::string>();
		asio::ip::address_v4 ipaddress = asio::ip::address_v4::from_string(ipaddress_str);				
		char ipaddress_string[19 + 1];
		ulong ll;
		ulong ip[4];	
		int i;
				
		ll = ipaddress.to_ulong();
												
		ip[3] = ((ll >> 24) & 0xFF);
		ip[2] = ((ll >> 16) & 0xFF);
		ip[1] = ((ll >> 8) & 0xFF);
		ip[0] = (ll & 0xFF);
				
		snprintf(ipaddress_string, 19, "?%c%03d%03d%03d%03d    \r", 'B' , ip[3], ip[2], ip[1], ip[0]);	
				
		std::string ip_address = ipaddress_string;
				
		std::cout << "PACKET OUT " << ip_address << std::endl;
												
	}									
}
	
	


/************************************************************
 * main method
 *
 * Version History:
 * Author               Date        Version  What was modified?
 * SAFE Engineering     6 Oct 2015  1.0.0    Original
 ************************************************************/
int main(int argc, char *argv[])
{
	std::cout << "START QRFL DATA DUMP LOG " << VERSION_STR << std::endl;
	
	std::string strLogPath = "/logs/web-app/public/log/";    
	//std::string strLogPath = "/home/debian/web-app/public/log/";
	
    bool bStdOut = false;
    bool bStdErr = false;
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            bStdOut = true;
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            bStdErr = true;
        }
	    else if (strcmp(argv[i], "-i") == 0)
	    {
		    bStdErr = true;
		    Init();
		    Read();
		    return 0;
	    }
    }
	
	if (aurizon::mkpath(strLogPath, 0744))
	{
		std::cerr << "Unable to create log path: " << strLogPath << std::endl;
		return 1;
	}
			 	
	SafeEngineering::Utils::Settings appSettings;
	if (SafeEngineering::Utils::LoadSettings(appSettings) == false)
	{
		std::cerr << "Main() function failed to load settings from json/text file" << std::endl;
		return -1;            
	}
	
	std::cout << "SITENAME : '" << appSettings.SiteName << "'" << std::endl;

    spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::milliseconds(5000));

    std::vector<spdlog::sink_ptr> sinks;
    if (bStdOut)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    }
    if (bStdErr)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stderr_sink_st>());
    }
	sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPath + "DebugStatusLog-" + appSettings.SiteName, "txt", 32 * 1024, 3, false));
    auto log = spdlog::create("status_log", begin(sinks), end(sinks));
    log->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%l] %v");
    // FIXME: level should be err normally
	log->set_level(spdlog::level::info);
	

    std::vector<spdlog::sink_ptr> test_sinks;
	test_sinks.push_back(std::make_shared<aurizon::DailyFileSink_st>(strLogPath, "DebugDataLog-" + appSettings.SiteName, "txt", 40, false));
    auto test_log = spdlog::create("dump_data_log", begin(test_sinks), end(test_sinks));
    test_log->set_pattern("%v");

    while (true)
    {
        log->info() << "Starting QRFL Data Dump Log [" << VERSION_STR << "]";

        asio::io_service ioService;

        aurizon::Controller controller(ioService);

        controller.start();

        std::error_code errorCode;
        bool hasError = false;
        try
        {
            ioService.run(errorCode);
            if (errorCode)
            {
                log->error() << "Controller stopped with an error: " << errorCode.message();
                hasError = true;
            }
            else
            {
	            log->info() << "Stopped";
            }
        }
        catch (const std::exception& ex)
        {
            log->error() << "Controller stopped with an exception: " << ex.what();
            hasError = true;
        }
        catch (...)
        {
            log->error() << "Controller stopped with unexpected exception";
            hasError = true;
        }

        log->flush();
        test_log->flush();
        sync();

        if (hasError)
        {
            return 2;
        }

        if (controller.getStopCode() != aurizon::Controller::StopCode::Restart)
        {
            return 0;
        }

        sleep(2);
    }
}
