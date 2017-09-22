/************************************************************
 * TestLogService.h
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		8 Nov 2016	1.0.0    Original
 ************************************************************/

#ifndef __TESTLOGSERVICE_H_
#define __TESTLOGSERVICE_H_

#include "asio.hpp"
#include "Timer.h"
#include <chrono>
#include <memory>
#include <string>

namespace aurizon
{

class NodeServer;

class TestLogService : public std::enable_shared_from_this<TestLogService>
{
private:
    TestLogService(asio::io_service &ioService, const std::string &strPort,
            uint32_t nBaud);

public:
    TestLogService(const TestLogService&) = delete;
    virtual ~TestLogService();

    TestLogService& operator=(const TestLogService&) = delete;

    void start();
    void stop();

    static std::shared_ptr<TestLogService> Instance(asio::io_service &ioService,
            const std::string &strPort, uint32_t nBaud);

private:
    void startConnection();
    bool openConnection(asio::serial_port& serialPort);

    void startRead();
    void handleRead(const std::error_code& errorCode, std::size_t nBytesReceived, int32_t nReadID);

    void startRestartTimer();
    void handleRestartTimer(const std::error_code& errorCode);

    void startRxTimer();
    void handleRxTimer(const std::error_code& errorCode);

    void stopTimer();

    void clearReadBuffer();

    void processBuffer();

    void addLog(const std::string& str);

    static const uint32_t MAX_BUFFER_SIZE = 4096;

    asio::io_service& m_ioService;
    std::shared_ptr<asio::serial_port> m_ptrSerialPort;
    Timer<std::chrono::steady_clock> m_timer;
    const std::string m_strPort;
    const uint32_t m_nBaud;

    uint8_t m_aReadBuffer[MAX_BUFFER_SIZE];
    std::string m_strBuffer;

    std::size_t m_nSearchIndex = 0;

    std::chrono::system_clock::time_point m_logTime;;

    int32_t m_nReadID = 0;

    bool m_bIsRunning = false;
    bool m_bRestarting = false;
};

} /* namespace aurizon */

#endif /* __TESTLOGSERVICE_H_ */
