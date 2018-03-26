/************************************************************
 * Controller.h
 * Main Data Dump Log Service Controller Module
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		26th Mar 2018	0.0.5    Official Release to Aurzion
 ************************************************************/

#ifndef __CONTROLLER_H_
#define __CONTROLLER_H_

#include "Timer.h"
#include "asio.hpp"
#include <chrono>
#include <memory>

#define VERSION_STR "V0R05"

namespace aurizon
{

class TestLogService;

// Run everything
// Listen to NodeServer events and take action when settings have changed
class Controller
{
public:
	Controller(asio::io_service &ioService, bool consoleDebug);
    virtual ~Controller();

    void start();
    void stop();

    enum class StopCode
    {
        None,
        Restart
    };

    StopCode getStopCode();

private:

    void startup();
    void shutdown();

    void handleSignal(const std::error_code& ec, int signalNumber);

    asio::io_service &m_ioService;
    Timer<std::chrono::steady_clock> m_timer;
    asio::signal_set m_signalSet;
    StopCode m_stopCode = StopCode::None;
    std::shared_ptr<TestLogService> m_ptrTestLogService;

};

} /* namespace aurizon */

#endif /* __CONTROLLER_H_ */
