/************************************************************
 * Controller.h
 * Main Data Dump Log Service Controller Module
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
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
	//Add in Termination Signals to Kill Process Service
    m_signalSet.add(SIGINT);
    m_signalSet.add(SIGTERM);
//#ifdef SIGQUIT
//    m_signalSet.add(SIGQUIT);
//#endif
	//Start Data Dump Test Log Service on UART 2 ttyS2 at 57600 Baud.
	m_ptrTestLogService = TestLogService::Instance(ioService, "/dev/ttyS2", 57600, consoleDebug);
	
}

Controller::~Controller()
{
}

//Start the Service
void Controller::start()
{
    
    startup();
    m_signalSet.async_wait(std::bind(&Controller::handleSignal, this, std::placeholders::_1, std::placeholders::_2));
	
}

//Stop the Service
void Controller::stop()
{
    m_stopCode = StopCode::None;
    shutdown();
}

//Get Stop Code - reason for stopping
Controller::StopCode Controller::getStopCode()
{
    return m_stopCode;
}


//Start up the Log
void Controller::startup()
{
	spdlog::get("status_log")->info() << "Startup";
    m_ptrTestLogService->start();

}

//Abort the Service and Shutdown	
void Controller::shutdown()
{
    std::error_code errorCode;
    m_signalSet.cancel(errorCode);
    m_timer.cancel(errorCode);
    m_ptrTestLogService->stop();
	m_ioService.stop();
}

//Handle System Termination (Abort) Signal
void Controller::handleSignal(const std::error_code& ec, int signalNumber)
{
    if (!ec)
    {
        m_stopCode = StopCode::None;
        shutdown();
    }
}

} /* namespace aurizon */
