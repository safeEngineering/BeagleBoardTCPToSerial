/************************************************************
 * TestLogService.cpp
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		8 Nov 2016	1.0.0    Original
 ************************************************************/

#include "TestLogService.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <sstream>
#include <iomanip>

namespace aurizon
{

std::shared_ptr<TestLogService> TestLogService::Instance(asio::io_service &ioService,
            const std::string &strPort, uint32_t nBaud)
{
    class make_shared_enable : public TestLogService
    {
    public:
        make_shared_enable(asio::io_service &ioService, const std::string &strPort, uint32_t nBaud) :
            TestLogService(ioService, strPort, nBaud) {}
    };
    return std::make_shared<make_shared_enable>(ioService, strPort, nBaud);
}

TestLogService::TestLogService(asio::io_service &ioService, const std::string &strPort, uint32_t nBaud) :
        m_ioService(ioService), m_timer(ioService), m_strPort(strPort), m_nBaud(nBaud)
{
}

TestLogService::~TestLogService()
{
}

static std::string timeString(const std::chrono::system_clock::time_point& tp);

void TestLogService::start()
{
    if (m_bIsRunning)
    {
        throw std::logic_error("TestLogService already started");
    }

    spdlog::get("test_log")->info() << timeString(std::chrono::system_clock::now()) << " Starting";

    m_bIsRunning = true;
    startConnection();
}

void TestLogService::stop()
{
    spdlog::get("test_log")->info() << timeString(std::chrono::system_clock::now()) << " Stopped";
    m_nReadID++;
    std::error_code errorCode;
    if (m_ptrSerialPort)
    {
        m_ptrSerialPort->close(errorCode);
        m_ptrSerialPort = nullptr;
    }
    m_strBuffer.clear();
    m_nSearchIndex = 0;
    stopTimer();
}

void TestLogService::startConnection()
{
    m_ptrSerialPort = std::make_shared<asio::serial_port>(m_ioService);
    if (openConnection(*m_ptrSerialPort))
    {
        clearReadBuffer();
        startRead();
    }
    else
    {
        m_ptrSerialPort = nullptr;
        startRestartTimer();
    }
}

bool TestLogService::openConnection(asio::serial_port& serialPort)
{
    std::error_code errorCode;

    errorCode = std::error_code();
    serialPort.open(m_strPort, errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    serialPort.set_option(asio::serial_port::baud_rate(m_nBaud), errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    serialPort.set_option(asio::serial_port::character_size(8), errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    serialPort.set_option(asio::serial_port::parity(asio::serial_port::parity::none), errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    serialPort.set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::one),
            errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    serialPort.set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::none),
            errorCode);
    if (errorCode)
    {
        std::error_code ec;
        serialPort.close(ec);
        return false;
    }

    return true;
}

void TestLogService::startRead()
{
    auto self = shared_from_this();
    m_nReadID++;
    int32_t nReadID = m_nReadID;
    m_ptrSerialPort->async_read_some(asio::buffer(m_aReadBuffer, MAX_BUFFER_SIZE),
            [self,nReadID](const std::error_code& errorCode, std::size_t nBytesReceived)
    {
        self->handleRead(errorCode, nBytesReceived, nReadID);
    });
}

void TestLogService::handleRead(const std::error_code& errorCode, std::size_t nBytesReceived, int32_t nReadID)
{
    if (errorCode == asio::error::operation_aborted || nReadID != m_nReadID)
    {
        return;
    }

    stopTimer();

    if (errorCode)
    {
        spdlog::get("log")->warn() << "TestLogService read error: " << errorCode.message();
        std::error_code ec;
        m_ptrSerialPort->close(ec);
        m_ptrSerialPort = nullptr;
        startRestartTimer();
        return;
    }

    if (nBytesReceived != 0)
    {
        if (m_strBuffer.empty())
        {
            m_logTime = std::chrono::system_clock::now();
        }
        m_strBuffer.insert(m_strBuffer.end(), m_aReadBuffer, m_aReadBuffer + nBytesReceived);

        while (true)
        {
            std::size_t nEndPos = m_strBuffer.find("\r\n\r\n", m_nSearchIndex);
            if (nEndPos == std::string::npos)
            {
                if (m_strBuffer.size() > 4096)
                {
                    addLog(m_strBuffer);
                    m_strBuffer.clear();
                    m_nSearchIndex = 0;
                }
                else if (m_strBuffer.size() >= 4)
                {
                    m_nSearchIndex = m_strBuffer.size() - 3;
                }
                break;
            }
            else
            {
                std::string str = m_strBuffer.substr(0, nEndPos + 4);
                addLog(str);
                m_strBuffer.erase(0, nEndPos + 4);
                m_nSearchIndex = 0;
                m_logTime = std::chrono::system_clock::now();
            }
        }
    }

    startRead();

    if (m_strBuffer.size())
    {
        startRxTimer();
    }
}

void TestLogService::startRestartTimer()
{
    if (m_bRestarting)
    {
        return;
    }

    m_bRestarting = true;
    std::error_code errorCode;
    m_timer.expires_after(std::chrono::milliseconds(5000), errorCode);
    m_timer.async_wait(
            std::bind(&TestLogService::handleRestartTimer, this, std::placeholders::_1));
}

void TestLogService::handleRestartTimer(const std::error_code& errorCode)
{
    if (errorCode == asio::error::operation_aborted)
    {
        return;
    }

    if (errorCode)
    {
        throw std::runtime_error("TestLogService restart timer: " + errorCode.message());
    }

    m_bRestarting = false;

    startConnection();
}

void TestLogService::startRxTimer()
{
    std::error_code errorCode;
    m_timer.expires_after(std::chrono::milliseconds(50), errorCode);
    m_timer.async_wait(
            std::bind(&TestLogService::handleRxTimer, this, std::placeholders::_1));
}

void TestLogService::handleRxTimer(const std::error_code& errorCode)
{
    if (errorCode == asio::error::operation_aborted)
    {
        return;
    }

    if (errorCode)
    {
        throw std::runtime_error("TestLogService rx timer: " + errorCode.message());
    }

    addLog(m_strBuffer);
    m_strBuffer.clear();
    m_nSearchIndex = 0;
}

void TestLogService::stopTimer()
{
    std::error_code errorCode;
    m_timer.cancel(errorCode);
}

void TestLogService::clearReadBuffer()
{
    if (m_ptrSerialPort)
    {
        tcflush(m_ptrSerialPort->native_handle(), TCIFLUSH);
    }
}

void TestLogService::addLog(const std::string& str)
{
#warning also send to aurizon_node
    spdlog::get("test_log")->info() << timeString(m_logTime) << "\r\n" << str;
}

static std::string timeString(const std::chrono::system_clock::time_point& tp)
{
    time_t t = std::chrono::system_clock::to_time_t(tp);
    tm* ptm = std::localtime(&t);
    char szDateBuffer[128];
    size_t nStartLength = strftime (szDateBuffer, 128, "[%d-%m-%Y %H:%M:%S.", ptm);

    auto duration = tp.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    size_t nEndLength = sprintf(&szDateBuffer[nStartLength], "%03u]", (unsigned int)millis);

    return std::string(szDateBuffer, nStartLength + nEndLength);
}

} /* namespace aurizon */
