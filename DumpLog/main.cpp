/************************************************************
 * main.cpp
 * Main Starting Point of Program
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
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

//Create Default Network JSON backup configuration and settings Files for testing purposes
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

//Read Backup Copy of Network JSON configuration File for testing purposes
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
	
	

//Main Program Starts Here
int main(int argc, char *argv[])
{
	std::cout << "START QRFL DATA DUMP LOG " << VERSION_STR << std::endl;
	
	std::string strLogPath = "/logs/web-app/public/log/";    
	std::string strLogPathEventLog = "/logs/web-app/public/eventlogs/";   
	std::string strLogPathFaultLog = "/logs/web-app/public/faultlogs/";   
	//std::string strLogPath = "/home/debian/web-app/public/log/";  //Removed for Final Version - use this path for testing only.
	
    bool bStdOut = false;
    bool bStdErr = false;
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)    //-o options Enabled STDOUT to console
        {
            bStdOut = true;
        }
        else if (strcmp(argv[i], "-e") == 0)  //-e options Enabled STDERR to console.
        {
            bStdErr = true;
        }
	    else if (strcmp(argv[i], "-i") == 0)  //-i options Create Test Network JSON Default Files.
	    {
		    bStdErr = true;
		    Init();
		    Read();
		    return 0;
	    }
    }
	
	//Create the Logging Paths
	if (aurizon::mkpath(strLogPath, 0744))
	{
		std::cerr << "Unable to create log path: " << strLogPath << std::endl;
		return 1;
	}
	
	if (aurizon::mkpath(strLogPathEventLog, 0744))
	{
		std::cerr << "Unable to create event log path: " << strLogPath << std::endl;
		return 1;
	}
	
	if (aurizon::mkpath(strLogPathFaultLog, 0744))
	{
		std::cerr << "Unable to create fault log path: " << strLogPath << std::endl;
		return 1;
	}
			 	
	//Load the Settings JSON File to get the system parameters - i.e Site Name for the Logs.
	SafeEngineering::Utils::Settings appSettings;
	if (SafeEngineering::Utils::LoadSettings(appSettings) == false)
	{
		std::cerr << "Main() function failed to load settings from json/text file" << std::endl;
		return -1;            
	}
	
	std::cout << "SITENAME : '" << appSettings.SiteName << "'" << std::endl;

	//Set Logging File System to refresh and flush only every 5 seconds.
    spdlog::set_async_mode(8192, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::milliseconds(5000));

	//Redirect STDOUT and STDERR to Console I/O if required
    std::vector<spdlog::sink_ptr> sinks;
    if (bStdOut)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
    }
    if (bStdErr)
    {
        sinks.push_back(std::make_shared<spdlog::sinks::stderr_sink_st>());
    }
	
	//Create a rotating 32K x 3 File Logging system for Debug Status Messages indicating this services health and activity.
	sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPath + "DebugStatusLog-" + appSettings.SiteName, "txt", 32 * 1024, 3, false));
    auto log = spdlog::create("status_log", begin(sinks), end(sinks));
    log->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%l] %v");
	log->set_level(spdlog::level::info);
	
	//Create a rotating 40 day File Logging system for the QRFL data dump logging contents
    std::vector<spdlog::sink_ptr> test_sinks;
	test_sinks.push_back(std::make_shared<aurizon::DailyFileSink_st>(strLogPath, "DebugDataLog-" + appSettings.SiteName, "txt", 40, false));
    auto test_log = spdlog::create("dump_data_log", begin(test_sinks), end(test_sinks));
    test_log->set_pattern("%v");
	
	//Create a rotating 128K x 3 File Logging system for QRFKL Fault Logs
	std::vector<spdlog::sink_ptr> event_sinks;
	event_sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPathEventLog + "Events-" + appSettings.SiteName, "txt", 128 * 1024, 3, false));
	auto event_log = spdlog::create("event_log", begin(event_sinks), end(event_sinks));
	event_log->set_pattern("[%d/%m/%Y %H:%M:%S] ,%v");	
	event_log->set_level(spdlog::level::info);
	
	//Create a rotating 128K x 3 File Logging system for QRFL Fault Logs	
	std::vector<spdlog::sink_ptr> fault_sinks;
	fault_sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPathFaultLog + "Faults-" + appSettings.SiteName, "txt", 128 * 1024, 3, false));
	auto fault_log = spdlog::create("fault_log", begin(fault_sinks), end(fault_sinks));
	fault_log->set_pattern("[%d/%m/%Y %H:%M:%S] ,%v");	
	fault_log->set_level(spdlog::level::info);

	//Start Main Program Loop
    while (true)
    {
        log->info() << "Starting QRFL Data Dump Log [" << VERSION_STR << "]";
	    
	    //Initialise Evetn and Fault Log CSV Headers
	    spdlog::get("event_log")->notice() << "QRFL_DATE,QRFL_TIME,START_IP,TRIP_X,TRIP_Y,OIL_SURGE_TRIP,OIL_TEMPERATURE_TRIP,WINDING_TEMPERATURE_TRIP,PRESSUREREL_TRIP,SPARE1_TRIP,SPARE2_TRIP,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,SPARE1_ALARM,BATTERY_CHARGER_ALARM,WINDING_TEMPERATURE_ALARM, BAG_RUPTURE_ALARM,OIL_TEMPERATURE_ALARM,GAS_ALARM,ALARM_W,ALARM_V,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED,RESERVED";
	    spdlog::get("fault_log")->notice() << "QRFL_DATE,QRFL_TIME,SUM FAULT CURRENTS 512 POINTS";

			    
        asio::io_service ioService;
	    
	    //Start the Data Dump Log Test Service        
	    aurizon::Controller controller(ioService, bStdOut);
		controller.start();

        std::error_code errorCode;
        bool hasError = false;
        try
        {
	        //Launch the Asynchronous Serial I/O
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

	    //Flush All Bufffers to write and remaining Log information.
        log->flush();
        test_log->flush();
	    event_log->flush();
	    fault_log->flush();        
        sync();

        if (hasError)
        {
            return 2;
        }

        if (controller.getStopCode() != aurizon::Controller::StopCode::Restart)
        {
            return 0;
        }

	    //Wait 2 seconds before stopping the program - give logs a chance to close.
        sleep(2);
    }
}
