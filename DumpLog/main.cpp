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
#include <cstdio>
#include <iostream>
#include <memory>

#define VERSION_STR "V0.2"

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
	if (aurizon::mkpath(strLogPath, 0744))
    {
        std::cerr << "Unable to create log path: " << strLogPath << std::endl;
        return 1;
    }

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
