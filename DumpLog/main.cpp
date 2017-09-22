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
#include <cstdio>
#include <iostream>
#include <memory>

/************************************************************
 * main method
 *
 * Version History:
 * Author               Date        Version  What was modified?
 * SAFE Engineering     6 Oct 2015  1.0.0    Original
 ************************************************************/
int main(int argc, char *argv[])
{
	std::string strLogPath = "/home/debian/web-app/public/log/";
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
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(strLogPath + "/logDump", "txt", 8 * 1024 * 1024, 3, false));
    auto log = spdlog::create("log", begin(sinks), end(sinks));
    log->set_pattern("[%d-%m-%Y %H:%M:%S.%e] [%l] %v");
    // FIXME: level should be err normally
    log->set_level(spdlog::level::warn);

    std::vector<spdlog::sink_ptr> test_sinks;
    test_sinks.push_back(std::make_shared<aurizon::DailyFileSink_st>(strLogPath, "dump_log", "txt", 40, false));
    auto test_log = spdlog::create("test_log", begin(test_sinks), end(test_sinks));
    test_log->set_pattern("%v");

    while (true)
    {
        log->error() << "Starting...";

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
                log->error() << "Stopped";
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