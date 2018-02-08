/************************************************************
 * Controller.cpp
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		9 Oct 2015	1.0.0    Original
 ************************************************************/

#include "Controller.h"
#include "FileUtil.h"
#include "spdlog/spdlog.h"
#include "TestLogService.h"

#include <sstream>
#include <regex>


// FIXME: compile with NDEBUG to remove asserts in production build
// XXX: need restart timer for command connections

namespace aurizon
{

Controller::Controller(asio::io_service &ioService, bool consoleDebug) :
        m_ioService(ioService),m_timer(ioService),m_signalSet(ioService)
{
    m_signalSet.add(SIGINT);
    m_signalSet.add(SIGTERM);
//#ifdef SIGQUIT
//    m_signalSet.add(SIGQUIT);
//#endif

	m_ptrTestLogService = TestLogService::Instance(ioService, "/dev/ttyS2", 57600, consoleDebug);
	
}

Controller::~Controller()
{
}

void Controller::start()
{
    
    startup();
    m_signalSet.async_wait(std::bind(&Controller::handleSignal, this, std::placeholders::_1, std::placeholders::_2));
	
}

void Controller::stop()
{
    m_stopCode = StopCode::None;
    shutdown();
}

Controller::StopCode Controller::getStopCode()
{
    return m_stopCode;
}



void Controller::startup()
{
	spdlog::get("status_log")->info() << "Startup";
    m_ptrTestLogService->start();

}

void Controller::shutdown()
{
    std::error_code errorCode;
    m_signalSet.cancel(errorCode);
    m_timer.cancel(errorCode);
    m_ptrTestLogService->stop();
	m_ioService.stop();
}

void Controller::handleSignal(const std::error_code& ec, int signalNumber)
{
    if (!ec)
    {
        m_stopCode = StopCode::None;
        shutdown();
    }
}

} /* namespace aurizon */
