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

#define DD_PACKET_DATA_LENGTH 16
#define DD_PACKET_STX '?'
#define DD_PACKET_ETX "\r"
#define DD_PACKET_IPADDR_CMD 'A'
#define DD_PACKET_NETMASK_CMD 'B'
#define DD_PACKET_GATEWAY_CMD 'C'
#define DD_PACKET_NTPADDR_CMD 'D'
#define DD_PACKET_DATETIME_CMD 'E'
#define DD_PACKET_NETWORKSTATUS_CMD 'F'
#define DD_PACKET_LENGTH (DD_PACKET_DATA_LENGTH+3)   //STX, CMD, DATA, ETX  

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
        m_ioService(ioService), m_timer(ioService), r_timer(ioService), m_strPort(strPort), m_nBaud(nBaud)
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

	spdlog::get("dump_data_log")->info() << timeString(std::chrono::system_clock::now()) << " Starting";

    m_bIsRunning = true;
    startConnection();
}

void TestLogService::stop()
{
	spdlog::get("dump_data_log")->info() << timeString(std::chrono::system_clock::now()) << " Stopped";
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
	r_timer.cancel(errorCode);
}

void TestLogService::startConnection()
{
    m_ptrSerialPort = std::make_shared<asio::serial_port>(m_ioService);
    if (openConnection(*m_ptrSerialPort))
    {
        clearReadBuffer();
        startRead();
	    startRestartTimer();
    }
    else
    {
        m_ptrSerialPort = nullptr;
        //startRestartTimer();
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
	    spdlog::get("status_log")->warn() << "TestLogService read error: " << errorCode.message();
        std::error_code ec;
        m_ptrSerialPort->close(ec);
        m_ptrSerialPort = nullptr;
        //startRestartTimer();
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
    r_timer.expires_after(std::chrono::milliseconds(5000), errorCode);
    r_timer.async_wait(
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

	//startConnection();
	
	
	std::string inputStr; 
	
	switch (sequenceCounter)
	{
	case 0:
		inputStr = "?A023111001101    \r";
		break;
	case 1:
		inputStr = "?B022222002102    \r";
		break;
	case 2:
		inputStr = "?C033133003103    \r";
		break;
	case 3:
		inputStr = "?D044144004104    \r";
		break;	
	default:
		inputStr = "?A000000000000    \r";
		break;	
	}
	
	sequenceCounter++;
	sequenceCounter = sequenceCounter % 4;;
	
	auto self = shared_from_this();
	m_ptrSerialPort->async_write_some(asio::buffer(inputStr, 19),		                		                
		[this, self](const std::error_code& errorCode, std::size_t nBytesReceived)
		{
			if (!errorCode)
			{
				std::cout << "Sent Bytes - " << nBytesReceived <<  std::endl;  				
			}
			else
			{
				std::cerr << "Sent" << std::endl;  
			}				                
		});		
	startRestartTimer();
	
}

void TestLogService::startRxTimer()
{
    std::error_code errorCode;
    m_timer.expires_after(std::chrono::milliseconds(50), errorCode);
    m_timer.async_wait(
            std::bind(&TestLogService::handleRxTimer, this, std::placeholders::_1));
}
	
std::string TestLogService::ReplaceAll(std::string str, const std::string& from, const std::string& to) 
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();  // Handles case where 'to' is a substring of 'from'
	}
	return str;
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
	
	std::string s = m_strBuffer;
	std::string delimiter = DD_PACKET_ETX;

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		if (ParseCommand(token))   
		{
			    			
		}
		s.erase(0, pos + delimiter.length());
	}
	
	m_strBuffer.clear();
    m_nSearchIndex = 0;
}

//Parse a Command of the format "?CAAABBBCCCDDD    "
bool TestLogService::ParseCommand(std::string str)
{
	const char *rxbuffer;
	int i;		
	uint16_t ip[4];	
	char IPAddressString[DD_PACKET_DATA_LENGTH + 1];
	
	
	if (str.length() != (DD_PACKET_LENGTH-1))  // Leave Out ETX in length comparison
	{
		return false;
	}
	
	rxbuffer = str.c_str();
	
	if (rxbuffer[0] != '?') return false;
								
	for (i = 2; i < 14; i++)   //Check all Numeric
	{
		if (!((rxbuffer[i] >= (uint8_t) '0') && (rxbuffer[i] <= (uint8_t) '9')))
		{			
			return false;	
		}
			
	}
		
	sscanf(&rxbuffer[2], "%03hd%03hd%03hd%03hd", &ip[0], &ip[1], &ip[2], &ip[3]);   //Parse IP address array as short ints to ensure it is less that or equal to the 16 bit number memory allocation.
	
	for(i = 0 ; i < 4 ; i++)
	{
		ip[i] &= 0x00FF;    //Truncate to 8 bit number;
	}
	
	snprintf(IPAddressString, DD_PACKET_DATA_LENGTH, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);		
		
				
	switch(rxbuffer[1])
	{
		case DD_PACKET_IPADDR_CMD:
			IP = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "IP - " << IP.to_string() <<  std::endl;
			break;
		case DD_PACKET_NETMASK_CMD:
			NETMASK = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "NETMASK - " << NETMASK.to_string() <<  std::endl;					
			break;
		case DD_PACKET_GATEWAY_CMD:
			GATEWAY = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "GATEWAY - " << GATEWAY.to_string() <<  std::endl;					
			break;
		case DD_PACKET_NTPADDR_CMD:
			NTPSERVER = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "NTPSERVER - " << NTPSERVER.to_string() <<  std::endl;					
			break;
		case DD_PACKET_DATETIME_CMD:
			break;
		case DD_PACKET_NETWORKSTATUS_CMD:
			break;
		default:
			return false;
	}
								
	return true;	
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
	spdlog::get("dump_data_log")->info() << timeString(m_logTime) << "\r\n" << str;
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
