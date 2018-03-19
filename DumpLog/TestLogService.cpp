/************************************************************
 * TestLogService.cpp
 *
 * Version History:
 * Author				Date		Version  What was modified?
 * SAFE	Engineering		8 Nov 2016	1.0.0    Original
 ************************************************************/

#include "TestLogService.h"
#include "Controller.h"
#include "spdlog/spdlog.h"
#include "NTP.hpp"
#include "PING.hpp"
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <json.hpp>
#include <unistd.h>
#include <stdio.h>


//QRFL Modes
#define QRFL_MODE_V23 0
#define QRFL_MODE_V23_LAN 1

//QRFL Configuration Bit Masks
#define CONFIG_DC_FILTER			0x00000001
#define MK3_RTU_FORMAT				0x00000002
#define V23_ONLINE					0x00000004
#define VT_ENABLED					0x00000020
#define IT_ENABLED					0x00000040
#define LOG_ONLINE					0x10000000


namespace aurizon
{

std::shared_ptr<TestLogService> TestLogService::Instance(asio::io_service &ioService,
		const std::string &strPort,
		uint32_t nBaud,
		bool consoleDebug)
{
    class make_shared_enable : public TestLogService
    {
    public:
        make_shared_enable(asio::io_service &ioService, const std::string &strPort, uint32_t nBaud, bool consoleDebug) :
            TestLogService(ioService, strPort, nBaud, consoleDebug) {}
    };
	return std::make_shared<make_shared_enable>(ioService, strPort, nBaud, consoleDebug);
}

TestLogService::TestLogService(asio::io_service &ioService, const std::string &strPort, uint32_t nBaud, bool consoleDebug) :
        m_ioService(ioService), m_timer(ioService), r_timer(ioService), m_strPort(strPort), m_nBaud(nBaud), m_NTP(ioService, consoleDebug, "8.8.8.8"), m_PING(ioService, consoleDebug, "8.8.8.8")
{
		int i;
	
		StdOutDebug = consoleDebug;
		sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
		m_NTP.m_NtpActive = 0;
		m_PING.m_PingActive = 0;	
	
		for (i = 0; i < MAX_NUM_SETTING_COMMANDS; i++)
		{
			receivedValidSettingsData[i] = false;
			previousSettingsReplyString[i] = "";
			currentSettingsReplyString[i] = "";
		}
	
}

TestLogService::~TestLogService()
{
}

static std::string timeString(const std::chrono::system_clock::time_point& tp);
	
	
void TestLogService::ReadNTPandGatewayIPAddresses()
	{
				
		// Load json file from current directory
		std::ifstream jsonFile("/home/debian/QRFL_Network_Configuration.json");	
		// JSON object
		nlohmann::json settingsjson;
		// Deserialize json object from text content
		
		jsonFile >> settingsjson;
	
		
		auto valueA = settingsjson.find("Gateway_Configuration");
		if (valueA != settingsjson.end()) 
		{				
			try
			{
			
				std::string ipaddress_str = valueA->get<std::string>();
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
				
				snprintf(ipaddress_string, 19, "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);	
							
				std::string ip_address = ipaddress_string;
				
				//std::cout << "GATEWAY IP ADDRESS " << ip_address << std::endl;
			
				m_PING.SetIPAddress(ip_address);
			}
			catch(const std::exception& ex)
			{
				
			}												
		}
		
		auto valueB = settingsjson.find("NTP_Configuration");
		if (valueB != settingsjson.end()) 
		{	
			try
			{
				std::string ipaddress_str = valueB->get<std::string>();
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
				
				snprintf(ipaddress_string, 19, "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);	
							
				std::string ip_address = ipaddress_string;
				
				//std::cout << "NTP IP ADDRESS " << ip_address << std::endl;
			
				m_NTP.SetIPAddress(ip_address);	
			}
			catch(const std::exception& ex)
			{
				
			}												
		}
	}
		

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
	m_NTP.StopNTP();
	m_PING.StopPing();
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
	    
	    ReadNTPandGatewayIPAddresses();
	    
	    // Initialize NTP operation
	    m_NTP.InitializeNTP();
	    // Initialize PING operation
	    m_PING.InitializePing();
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
	            
				/*            
	            //Find any commands
	            std::string s = m_strBuffer;
	            std::string delimiterB = DD_PACKET_ETX_STR;
	            std::string delimiterA = DD_PACKET_STX_STR;
	            
	            size_t posA = 0;
	            size_t posB = 0;
	            std::string token;
	            while ((posB = s.find(delimiterB)) != std::string::npos) {      //Contains ETX
		            token = s.substr(0, posB);	
		            if ((posA = token.find(delimiterA)) != std::string::npos)   //Contains STX
		            {
			            if ((token.size() - posA) >= (DD_PACKET_LENGTH-1))  //Valid Packet -1 (leave out ETX)
			            {				            			            
				            token = token.substr(posA, token.size() - posA);	            	
				            //std::cout << "'" << token << "'" << std::endl;
				            if (ParseCommand(token))   
				            {
			    			
				            }
			            }
					}
		            s.erase(0, posB + delimiterB.length());
	            }
	            
	            
	            //Find any commands
	            s = m_strBuffer;
	            delimiterB = "\r\n";
	            delimiterA = ":U";
	            
	            posA = 0;
	            posB = 0;
	            while ((posB = s.find(delimiterB)) != std::string::npos) {
		
		            token = s.substr(0, posB);	
		            if ((posA = token.find(delimiterA)) != std::string::npos)   
			            {
				            if ((token.size() - posA) >= 2)  //Valid Packet Data must have at least 2 characetrs
					            {				            			            
						            token = token.substr(posA, token.size() - posA);	            	
						            std::cout << "{" << token << "}" << std::endl;						             
						            //Process this command
					            }
			            }
		            s.erase(0, posB + delimiterB.length());
	            }
				*/
	            
	            //Find any packet commands	            
				ParseDebugLogText(m_strBuffer, DD_PACKET_STX_STR, DD_PACKET_ETX_STR, true);
	
	            //Find any non standard commands	            
	            ParseDebugLogText(m_strBuffer, ":", "\r\n", false);
		            
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

void TestLogService::TestLogService::startRestartTimer()
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

void TestLogService::SimulateDatatoLog(int counter)		
{
		std::string s;
		
		switch (counter)
		{
			case 1:
				s= std::string("?E180306175613    @86B6\r\n");
				s+=std::string(":Z17:56:06.182,ALDOGA AT,0,150\r\n");
				s+=std::string(":A00000000000000000000000000000000\r\n");
				s+=std::string(":B00110011010000000000000000010001\r\n");
				s+=std::string("?g1000000000000000@57E5\r\n");
				s+=std::string("?h0000000000000000@0A7F\r\n");
				s+=std::string(":C0.08,0.08,0.07,0.08\r\n");
				s+=std::string(":D1,42,15,23,9,15,9\r\n");
				s+=std::string(":E916,0,0,0,0,0,0\r\n");
				s+=std::string(":F-46.8,-46.8\r\n");
				s+=std::string(":G0\r\n");
				s+=std::string(":H00000010\r\n");
				s+=std::string(":I0000010000000001\r\n");
				s+=std::string(":J0000000000000001\r\n");
				s+=std::string(":K00010400\r\n");
				s+=std::string(":L17:56:13.347,17:56:13.351\r\n");
				s+=std::string(":V00,00,00,00,00,00,00\r\n");
				s+=std::string(":W00,00,00,00,00,00,00\r\n");
				s+=std::string(":M1,1,1,1,2,1,1,1,0,0,2,2,1,2,0,2,1,3,2,1,1,1,0,1,2,1,1,1,1,3,2,1,1,2,1,1,1,2,1,0,2,2,2,2,1,1,2,3,1,1,0,1,1,1,1,0,0,0,0,0,1,1,0,1,1,0,2,2,1,1,1,-1,1,0,1,-1,2,0,0,1,1,1,1,1,2,1,0,2,1,2,2,1,1,1,0,1,2,0,2,1,2,4,2,1,1,3,2,1,0,2,2,2,0,2,2,1,0,1,1,0,1,1,1,0,0,0,0,2,2,1,1,2,2,1,1,2,2,2,1,2,3,1,0,1,2,0,1,1,2,2,1,2,1,1,1,1,0,1,0,1,2,1,1,1,0,1,2,3,1,2,3,1,2,2,0,1,0,2,1,0,0,2,1,1,1,2,1,1,0,2,0,-1,1,1,2,1,0,1,0,0,1,1,2,1,0,1,2,1,2,1,1,0,2,2,0,2,1,2,3,1,0,0,1,2,2,1,0,1,2,4,3,1,2,1,2,2,2,2,1,1,1,2,2,0,1,1,2,1,1,1,2,2,2,2,1,1,2,2,2,1,1,3,2,2,2,2,2,0,0,1,1,1,0,2,2,1,2,1,2,0,1,2,1,2,0,1,0,0,1,1,1,1,2,1,1,1,1,0,1,1,1,-1,1,1,1,2,2,1,2,1,1,2,0,2,1,1,0,2,3,0,2,1,2,1,1,2,2,0,1,1,2,1,1,2,0,0,1,2,1,1,1,1,1,1,1,1,2,3,2,1,1,1,0,1,2,3,2,1,1,3,3,2,2,1,1,1,1,1,2,2,2,1,0,0,0,2,1,1,2,1,1,2,1,1,0,1,1,1,0,0,1,3,2,0,1,2,2,0,0,0,1,2,2,0,1,0,1,2,2,1,0,2,0,3,0,1,1,1,0,0,0,1,1,2,2,0,1,1,1,2,2,1,1,2,2,1,1,1,1,0,2,2,1,0,2,2,2,3,2,0,0,0,0,1,2,2,1,0,2,1,0,2,1,1,1,3,1,2,2,2,0,2,1,0,0,1,3,2,2,2,3,2,3,1,-1,1,1,3,2,0,2,-1,0,1,1,1,1,2,1,2,1,1,1,1,1,3,2,2,0,1,1,0,\r\n");
				s+=std::string(":N0000,0000,0000,0000,0000,0000,0000,0000,-001,-008,-019,-032,-046,-061,-076,-090,-102,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":O0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":P0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":Q0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":R0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":S0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":T0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000,0000\r\n");
				s+=std::string(":U01,01,01,01,0203,01,50000075,0005,2ee0,04,03,00,0728,0a,0060,3c,01f4,0064\r\n");
				s+=std::string(":X0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\r\n\r\n");			
			break;
			
			case 2:
				s =  std::string("?E180306175547    @9433\r\n");
				s += std::string("?G0010000000000000@B8A2\r\n");
			break;
			
			case 3:
				s =  std::string("?E180306175547    @9433\r\n");
				s += std::string("?G0010000000000000@B8A2\r\n");
			break;
												
			case 4:				
				s =  std::string("?iALDOGA AT       @EA2B\r\n");
				s += std::string("?j                @6EEA\r\n");
				s += std::string("?k1000007500010203@0C19\r\n");
				s += std::string("?l00052EE004030064@E588\r\n");
				s += std::string("?m01F400600A0A003C@6A6B\r\n");
				s += std::string("?n001E009601070000@92A3\r\n");
				s += std::string("?o0100000001800000@630E\r\n");
				s += std::string("?p2552552550000000@8C1C\r\n");
				s += std::string("?q0100000001380000@59DC\r\n");
				s += std::string("?r2162390350080000@C511\r\n");				
			break;
			
			case 5:
				s =  std::string("?E180306175547    @9433\r\n");
				s += std::string("?G1000000000000000@6BE7\r\n");
			break;
			
			case 6:
				s =  std::string("?E180306175547    @9433\r\n");
				s += std::string("?G1000000000000000@6BE7\r\n");
			break;
						
			default:
				s =  std::string("?E180306174638    @A6DA\r\n");
			break;			
		}
		
		//Find any packet commands	            
		ParseDebugLogText(s, DD_PACKET_STX_STR, DD_PACKET_ETX_STR, true);
	
		//Find any non standard commands	            
		ParseDebugLogText(s, ":", "\r\n", false);
		
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
		
	debugCounter++;     
	
	//SimulateDatatoLog(debugCounter);   //AE SIMULATOR
		
	if ((debugCounter % 10) == 0)
	{
		inputStr = "";
		
		if (debugCounter == 10)
		{
			std::cout << "REQUEST SETTINGS" << std::endl;
			SendBasicCommand(DD_PACKET_SETTINGS_REQ_CMD, inputStr);				
		}
		if (debugCounter == 100)
		{
			debugCounter = 0;   // Start Counter Again.
		}
		
/*		
		if (debugCounter == 20)
		{
			std::cout << "REQUEST NETWORK "  << std::endl;	
			SendBasicCommand(DD_PACKET_REQ_NETWORK_STATUS, inputStr);		
		}
		
		if (debugCounter == 30)
		{
			std::cout << "REQUEST RESET "  << std::endl;	
			SendBasicCommand(DD_PACKET_REQ_RESET_CMD, inputStr);	
			debugCounter = 0;
		}
*/			
	}
	else
	{
		
	
		if ((fabs(QRFLTimeDifference) > TIME_SYNC_TOLERANCE_SECS) && (QRFLTimeDifferenceBlankOutCounter == 0) && (m_NTP.m_NtpActive >= NTP_POLL_ACCEPTABLE_ONLINE_COUNT))  //If time out of synch and NTP Server is active
			{
				inputStr = "";
				GetDateTimeQRFLSynch(inputStr);
				QRFLTimeDifferenceBlankOutCounter = TIME_SYNC_REPEAT_LOCKOUT;
			}
		else
		{
		
			if (QRFLTimeDifferenceBlankOutCounter > 0)
			{
				QRFLTimeDifferenceBlankOutCounter--;
			}
		
			switch (sequenceCounter)
			{
			case DD_PACKET_IPADDR_CMD:
				inputStr = "";
				LoadNetworkConfigJSONFile(DD_PACKET_IPADDR_CMD, inputStr);
				sequenceCounter = (int16_t) DD_PACKET_NETMASK_CMD;
				break;
			case DD_PACKET_NETMASK_CMD:
				inputStr = "";
				LoadNetworkConfigJSONFile(DD_PACKET_NETMASK_CMD, inputStr);
				sequenceCounter = (int16_t) DD_PACKET_GATEWAY_CMD;
				break;
			case DD_PACKET_GATEWAY_CMD:
				inputStr = "";
				LoadNetworkConfigJSONFile(DD_PACKET_GATEWAY_CMD, inputStr);
				sequenceCounter = (int16_t) DD_PACKET_NTPADDR_CMD;
				break;
			case DD_PACKET_NTPADDR_CMD:
				inputStr = "";
				LoadNetworkConfigJSONFile(DD_PACKET_NTPADDR_CMD, inputStr);
				sequenceCounter = (int16_t) DD_PACKET_VERSION_CMD;
				break;	
			case DD_PACKET_VERSION_CMD:
				inputStr = "";
				LoadVersion(DD_PACKET_VERSION_CMD, inputStr);				
				ReadNTPandGatewayIPAddresses(); 						//Update the IP and NTP here as good as any place
				sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
				break;
			default:
				inputStr = "";
				sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
				break;	
			}
		}
		
	}
		
	if (inputStr != "")
	{			
		auto self = shared_from_this();
		if (StdOutDebug) std::cout << "Out Str - " << inputStr <<  std::endl;  
		m_ptrSerialPort->async_write_some(asio::buffer(inputStr, DD_PACKET_LENGTH),		                		                
			[this, self](const std::error_code& errorCode, std::size_t nBytesReceived)
			{
				if (!errorCode)
				{
					//std::cout << "Sent Bytes - " << nBytesReceived <<  std::endl;  									
	
				}
				else
				{
					std::cerr << "Sent" << std::endl;  
				}				                
			});		
	}
	BlinkLED();
	startRestartTimer();
	
}

void TestLogService::startRxTimer()
{
    std::error_code errorCode;
    m_timer.expires_after(std::chrono::milliseconds(500), errorCode);
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

	
bool TestLogService::ParseDebugLogText(std::string m_strBuffer, std::string delimiterStart, std::string delimiterEnd, bool isPacketFormat)
{
		
	std::string s = m_strBuffer;
	            
	size_t posA = 0;
	size_t posB = 0;
	std::string token;
	size_t minimumSize;
	bool found = false;
	
	if (isPacketFormat)
	{
		
		minimumSize = (DD_PACKET_LENGTH - 1); 
	}
	else
	{
		minimumSize = 2;		
	}
			
	posA = 0;
	posB = 0;
	
	while ((posB = s.find(delimiterEnd)) != std::string::npos) {
		
		token = s.substr(0, posB);	
		if ((posA = token.find(delimiterStart)) != std::string::npos)   
		{
			if ((token.size() - posA) >= minimumSize)  //Valid Packet Data must be at least correct size
			{				            			            
				token = token.substr(posA, token.size() - posA);
				if (isPacketFormat)
				{
					//Process packet command
					if(StdOutDebug) std::cout << "'" << token << "'" << std::endl;
					if (ParseCommand(token))   
					{
						found = true;
					}	
				}
				else
				{	
					//Process non packet command
					if(StdOutDebug) std::cout << "{" << token << "}" << std::endl;						
					if (ParseText(token))   
					{
						found = true;
					}										
				}
			}
		}
		s.erase(0, posB + delimiterEnd.length());
	}
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
		
	//Find any packet commands	            
	ParseDebugLogText(m_strBuffer, DD_PACKET_STX_STR , DD_PACKET_ETX_STR, true);
	
	//Find any non standard commands	            
	ParseDebugLogText(m_strBuffer, ":", "\r\n", false);
	
	m_strBuffer.clear();
    m_nSearchIndex = 0;
}
	

uint16_t TestLogService::crc16_ccitt(const char *buf, int len)
{
	volatile int counter;
	volatile uint16_t crc = 0;
	uint16_t idx;
        
	for (counter = 0; counter < len; counter++)
	{
		idx = ((crc >> 8) ^ ((uint16_t) buf[0])) & 0x00FF;
		crc = (crc << 8) ^ crc16tab[idx];
		buf++;
	}
	return crc;
}
	
std::string TestLogService::GetTimeAsString(std::string formatString, time_t theTime)
{
	struct tm *timeinfo;
	timeinfo = localtime(&theTime);

	char buffer[100];
	strftime(buffer, 100, formatString.c_str(), timeinfo);
	std::string result(buffer);
	return result;
}
	
//Parse a Command of the format ":M    ...  "
bool TestLogService::ParseText(std::string str)
{
	const char *rxbuffer;
	std::string s;
	
	if (str.length() < 2)  
	{
		return false;
	}
	
	rxbuffer = str.c_str();
	
	if (rxbuffer[0] != ':')
	{
		return false;
	}
	
	switch (rxbuffer[1])
	{
		case 'M':
			s = str;
			s.erase(0, 2);   //Remove :M prefix
			spdlog::get("fault_log")->info() << GetTimeAsString("%d/%m/%Y,%H:%M:%S,", QRFLTime) << s;
			break;
		default:
		break;
	}
	return true;
}
//Parse a Command of the format "?CAAABBBCCCDDD    "
bool TestLogService::ParseCommand(std::string str)
{
	const char *rxbuffer;
	int i;		
	uint16_t ip[4];	
	uint16_t crc = 0;
	uint16_t crc_calc = 0;
	uint16_t hr, min, sec, date, month, year;
	struct tm time_str;
	std::string s = "";
	bool ret;
											
	char IPAddressString[DD_PACKET_DATA_LENGTH + 1];
		
	if (str.length() != (DD_PACKET_LENGTH-1))  // Leave Out ETX in length comparison
	{
		return false;
	}
			
	rxbuffer = str.c_str();
	
	if (rxbuffer[0] != DD_PACKET_STX) return false;   //Check for STX
	
	if (rxbuffer[(DD_PACKET_DATA_LENGTH + 2)] != DD_PACKET_CRC_TAG) return false;   // Check for CRC
								
	for (i = 2; i < (DD_PACKET_DATA_LENGTH + 2); i++)   //Check all Numeric or Spaces or Alpha
	{
		if (!(((rxbuffer[i] >= (uint8_t) '0') && (rxbuffer[i] <= (uint8_t) '9')) || (rxbuffer[i] == (uint8_t) ' ') || ((rxbuffer[i] >= (uint8_t) 'A') && (rxbuffer[i] <= (uint8_t) 'Z'))))
		{			
			return false;	
		}
			
	}
			
	for (i = (DD_PACKET_DATA_LENGTH + 2 + 1); i < (DD_PACKET_DATA_LENGTH + 2 + DD_PACKET_CRC_LENGTH); i++)   //Check all HEX but skip the CRC TAG @ (+1)
	{
		if (!(((rxbuffer[i] >= (uint8_t) '0') && (rxbuffer[i] <= (uint8_t) '9')) || ((rxbuffer[i] >= (uint8_t) 'A') && (rxbuffer[i] <= (uint8_t) 'F')) ))
		{			
			return false;	
		}
			
	}
	
	crc_calc = crc16_ccitt(rxbuffer, (DD_PACKET_DATA_LENGTH + 2));    //include STX and CMD
	
	sscanf(&rxbuffer[DD_PACKET_DATA_LENGTH + 2 + 1], "%04hX", &crc);     //Parse CRC HEX but skip the CRC TAG @ (+1)
	
	if(crc_calc != crc)
	{
		std::cout << "Check Sum Error " << crc_calc <<  " : " << crc << std::endl;		
		return false;
	}
			
	if ((rxbuffer[1] == DD_PACKET_IPADDR_CMD) || (rxbuffer[1] == DD_PACKET_NETMASK_CMD) || (rxbuffer[1] == DD_PACKET_GATEWAY_CMD) || (rxbuffer[1] == DD_PACKET_NTPADDR_CMD))   //i.e This is an IP Address Command
	{			
		sscanf(&rxbuffer[2], "%03hd%03hd%03hd%03hd", &ip[0], &ip[1], &ip[2], &ip[3]);    //Parse IP address array as short ints to ensure it is less that or equal to the 16 bit number memory allocation.	
		for(i = 0 ; i < 4 ; i++)
		{
			ip[i] &= 0x00FF;     //Truncate to 8 bit number;
		}	
		snprintf(IPAddressString, DD_PACKET_DATA_LENGTH, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);		
	}
		
					
	switch(rxbuffer[1])
	{
		case DD_PACKET_IPADDR_CMD:
			validNetworkSettingsRxState |= 0x01;
			IP = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "IP - " << IP.to_string() <<  std::endl;			
			//UpdateNetworkSettingsJSONFile(DD_PACKET_IPADDR_CMD, IP.to_string());
			break;
		case DD_PACKET_NETMASK_CMD:
			validNetworkSettingsRxState |= 0x02;
			NETMASK = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "NETMASK - " << NETMASK.to_string() <<  std::endl;		
			//UpdateNetworkSettingsJSONFile(DD_PACKET_NETMASK_CMD, NETMASK.to_string());
			break;
		case DD_PACKET_GATEWAY_CMD:
			validNetworkSettingsRxState |= 0x04;
			GATEWAY = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "GATEWAY - " << GATEWAY.to_string() <<  std::endl;	
			//UpdateNetworkSettingsJSONFile(DD_PACKET_GATEWAY_CMD, GATEWAY.to_string());
			break;
		case DD_PACKET_NTPADDR_CMD:
			validNetworkSettingsRxState |= 0x08;
			NTPSERVER = asio::ip::address_v4::from_string(IPAddressString);
			std::cout << "NTPSERVER - " << NTPSERVER.to_string() <<  std::endl;					
			//UpdateNetworkSettingsJSONFile(DD_PACKET_NTPADDR_CMD, NTPSERVER.to_string());	
			if(validNetworkSettingsRxState == 0x0F)
			{
				CreateNetworkSettingsJSONFile();			
			}
			validNetworkSettingsRxState = 0x00;
			break;
		case DD_PACKET_DATETIME_CMD:
			sscanf(&rxbuffer[2], "%02hd%02hd%02hd%02hd%02hd%02hd", &year, &month, &date, &hr, &min, &sec);       //Parse Date Time as short ints to ensure it is less that or equal to the 16 bit number memory allocation.	
			time_str.tm_year = year + 100; //Add 100 as tm_year is number of years since 1900
			time_str.tm_mon = month - 1;  //Subtract one as tm_mon is 0 .. 11
			time_str.tm_mday = date;
			time_str.tm_hour = hr;
			time_str.tm_min = min;
			time_str.tm_sec = sec;
			if (mktime(&time_str) != -1)
			{	
				QRFLTime = mktime(&time_str);
				
				time_t systemTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				
				QRFLTimeDifference = difftime(systemTime, QRFLTime);
				
				if (StdOutDebug) std::cout << "QRFL TIME - " << date << "-" << month << "-" << year << " " << hr << ":" << min << ":" << sec <<  "{" << QRFLTimeDifference << "}" << std::endl;						
												
			}
			else
			{
				QRFLTimeDifference = 0;
			}
			break;
		case DD_PACKET_TRIP_START_CHANGE_CMD:
			if (StdOutDebug) std::cout << "QRFL TRIP CHANGE" << std::endl;
			tripLogDataString = "";
			for (i = 2; i < (DD_PACKET_DATA_LENGTH + 2); i++)   //Check all Numeric or Spaces or Alpha
			{
				if (rxbuffer[i] ==  (uint8_t) '0')		
				{
					tripLogDataString += std::string("0,");
				}
				else
				{
					tripLogDataString += std::string("1,");
				}
			}			
			tripLogDataString.erase(tripLogDataString.length() - 1, 1);    //Erase last ','			
			if(tripLogDataString != prevTripLogDataString)
			{			
				spdlog::get("event_log")->info() << GetTimeAsString("%d/%m/%Y,%H:%M:%S,", QRFLTime) << tripLogDataString << "," << alarmLogDataString;
				prevTripLogDataString = tripLogDataString;
			}		
			break;
		case DD_PACKET_ALARMS_CHANGE_CMD:
			if (StdOutDebug) std::cout << "QRFL ALARM CHANGE" << std::endl;	
			alarmLogDataString = "";
			for (i = 2; i < (DD_PACKET_DATA_LENGTH + 2); i++)   //Check all Numeric or Spaces or Alpha
			{
				if (rxbuffer[i] ==  (uint8_t) '0')		
				{
					alarmLogDataString += std::string("0,");
				}
				else
				{
					alarmLogDataString += std::string("1,");
				}
			}			
			alarmLogDataString.erase(alarmLogDataString.length() - 1, 1);       //Erase last ','			
			if(alarmLogDataString != prevAlarmLogDataString)
			{			
				spdlog::get("event_log")->info() << GetTimeAsString("%d/%m/%Y,%H:%M:%S,", QRFLTime) << tripLogDataString << "," << alarmLogDataString;				
				prevAlarmLogDataString = alarmLogDataString;
			}		
			break;		
		case DD_PACKET_TRIP_START_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL TRIP STATUS" << std::endl;
			break;			
		case DD_PACKET_ALARMS_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL ALARM STATUS" << std::endl;
			break;
		case DD_PACKET_SETTINGS1_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 1 DATA" << std::endl;			
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 1 Parse Error"; }}		
			receivedValidSettingsData[0] = true;
			currentSettingsReplyString[0] = str;
			break;
		case DD_PACKET_SETTINGS2_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 2 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 2 Parse Error"; }}
			receivedValidSettingsData[1] = true;
			currentSettingsReplyString[1] = str;
			break;
		case DD_PACKET_SETTINGS3_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 3 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 3 Parse Error"; }}
			receivedValidSettingsData[2] = true;
			currentSettingsReplyString[2] = str;
			break;
		case DD_PACKET_SETTINGS4_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 4 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 4 Parse Error"; }}
			receivedValidSettingsData[3] = true;
			currentSettingsReplyString[3] = str;
			break;
		case DD_PACKET_SETTINGS5_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 5 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 5 Parse Error"; }}
			receivedValidSettingsData[4] = true;
			currentSettingsReplyString[4] = str;
			break;
		case DD_PACKET_SETTINGS6_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 6 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 6 Parse Error"; }}
			receivedValidSettingsData[5] = true;
			currentSettingsReplyString[5] = str;
			break;
		case DD_PACKET_SETTINGS7_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 7 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 7 Parse Error"; }}
			receivedValidSettingsData[6] = true;
			currentSettingsReplyString[6] = str;
			break;
		case DD_PACKET_SETTINGS8_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 8 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 8 Parse Error"; }}
			receivedValidSettingsData[7] = true;
			currentSettingsReplyString[7] = str;
			break;
		case DD_PACKET_SETTINGS9_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 9 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 9 Parse Error"; }}
			receivedValidSettingsData[8] = true;
			currentSettingsReplyString[8] = str;
			break;
		case DD_PACKET_SETTINGS10_STATUS_CMD:
			if (StdOutDebug) std::cout << "QRFL SETTINGS 10 DATA" << std::endl;
			ret = ProcessSettingsPacket(rxbuffer);     
			if (!ret) { if (StdOutDebug) {std::cout << "QRFL Settings 10 Parse Error"; }}
			receivedValidSettingsData[9] = true;
			currentSettingsReplyString[9] = str;
			CreateParameterSettingsJSONFile();
			break;
		default:
			return false;
	}
								
	return true;	
}

	
	void TestLogService::UpdateNetworkSettingsJSONFile(uint8_t ip_address_type, std::string ip_address)
	{
		
		try
		{
						
			std::string json_tag;
			
			// Load json file from current directory
			std::ifstream jsonFile("/home/debian/QRFL_Network_Settings.json", std::ios::in);
			// JSON object
			nlohmann::json settingsjson;
			// Deserialize json object from text content
			jsonFile >> settingsjson;
									
			switch (ip_address_type)
			{							
				case DD_PACKET_IPADDR_CMD:
					json_tag = "IP";
					break;
				case DD_PACKET_NETMASK_CMD:
					json_tag = "Mask";			
					break;
				case DD_PACKET_GATEWAY_CMD:
					json_tag = "Gateway";				
					break;
				case DD_PACKET_NTPADDR_CMD:
					json_tag = "NTP_IP";			
					break;
				default:
					return;										
			}
			
						
			auto value = settingsjson.find(json_tag);
			if (value != settingsjson.end()) 
			{
				
				std::cout << "SETTINGS [" << ip_address << "]" << std::endl;
				settingsjson[json_tag] = ip_address;	            	            
				std::ofstream outfile("/home/debian/QRFL_Network_Settings.json", std::ios::out);
				outfile << settingsjson;
				outfile.flush();
				outfile.close();
			}									
		}
		catch(nlohmann::json::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return;
		}
		catch(const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return;
		}    
	}
	
	void TestLogService::CreateNetworkSettingsJSONFile()
	{		
		try
		{
									
			std::string json_tag;
			
			nlohmann::json settingsjson =
			{						
				{ "IP", "10.0.0.100" },
				{ "Mask", "255.255.255.0" },
				{ "Gateway", "10.0.0.138" },
				{ "NTP_IP", "216.239.35.4" },
				{ "DNS", "8.8.8.8" }
			};
						
			settingsjson["IP"] = IP.to_string();	            	            							
			settingsjson["Mask"] = NETMASK.to_string();	            	            
			settingsjson["Gateway"] = GATEWAY.to_string();	            	            
			settingsjson["NTP_IP"] = NTPSERVER.to_string();	    
			
			//First Time running so Initialise old Details
			if((oldIPAddress == "") || (oldGatewayAddress == "") || (oldNetmaskAddress == "") || (oldNTPServerAddress == ""))
			{
				oldDetailsMatchCounter = 1;   //Adjust counter to reflect first lot of data recieved
				oldIPAddress = IP.to_string();
				oldGatewayAddress = GATEWAY.to_string();
				oldNetmaskAddress = NETMASK.to_string();
				oldNTPServerAddress = NTPSERVER.to_string();
				std::cout << "INIT SETTINGS FILE [IP:" << IP.to_string() << "][MASK:" << NETMASK.to_string() << "][GWAY:" << GATEWAY.to_string() << "][NTP:" << NTPSERVER.to_string() << "]" << std::endl;			
				return;
			}
			
			if ((oldIPAddress == IP.to_string()) && (oldGatewayAddress == GATEWAY.to_string()) && (oldNetmaskAddress == NETMASK.to_string()) && (oldNTPServerAddress == NTPSERVER.to_string()))
			{
				//If there are two consecutive matches of any of the details of the Network Settings - Update the settings file and start the re-boot sequence
				oldDetailsMatchCounter++;
				std::cout << "SAME[" << IP.to_string() << "][MASK:" << NETMASK.to_string() << "][GWAY:" << GATEWAY.to_string() << "][NTP:" << NTPSERVER.to_string() << "]" << std::endl;											
				if (oldDetailsMatchCounter == 2)	
				{									
					std::cout << "CREATE SETTINGS FILE [IP:" << IP.to_string() << "][MASK:" << NETMASK.to_string() << "][GWAY:" << GATEWAY.to_string() << "][NTP:" << NTPSERVER.to_string() << "]" << std::endl;						
					std::ofstream outfile("/home/debian/QRFL_Network_Settings.json", std::ios::out);
					outfile << settingsjson;
					outfile.flush();
					outfile.close();
				}						
				else if (oldDetailsMatchCounter > 2)
				{
					oldDetailsMatchCounter = 0;
					system("/home/debian/QRFL_Boot_Script.sh");	
					oldIPAddress = "";
					oldGatewayAddress = "";
					oldNetmaskAddress = "";
					oldNTPServerAddress = "";
				}
			}
			else
			{
				//Settings Differ Start Again.
				oldDetailsMatchCounter = 0;				
				std::cout << "MISMATCH[" << IP.to_string() << "][MASK:" << NETMASK.to_string() << "][GWAY:" << GATEWAY.to_string() << "][NTP:" << NTPSERVER.to_string() << "]" << std::endl;																						
				oldIPAddress = "";
				oldGatewayAddress = "";
				oldNetmaskAddress = "";
				oldNTPServerAddress = "";
			}
																					
		}
		catch (nlohmann::json::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return;
		}    
	}
	
	bool TestLogService::SendBasicCommand(uint8_t command_type, std::string& command)	
	{
									
		char cmd_string[DD_PACKET_LENGTH + 2];
		
						
		snprintf(cmd_string, (DD_PACKET_LENGTH + 1), "?%c                \r", command_type);      //PickOut Numbers x from VERSION STR Format VxRxx and Construct the command string first without the check sum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
																				
		uint16_t crc_calc = crc16_ccitt(cmd_string, (DD_PACKET_DATA_LENGTH + 2));          //calculate the checksum include STX and CMD
				
		snprintf(cmd_string, (DD_PACKET_LENGTH + 1), "?%c                @%04X\r", command_type, crc_calc);         	//PickOut Numbers x from VERSION STR Format VxRxx and Reconstruct the command string this time with the checksum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
								
		command = cmd_string;
		
		//std::cout << "VER PACKET OUT [" << version << "]" << std::endl;
		
		return true;		
	}
	
	bool TestLogService::LoadVersion(uint8_t ip_address_type, std::string& version)	
	{
						
		uint8_t activeState = ' ';		
		char version_string[DD_PACKET_LENGTH + 2];
		char VERSION_STR_CHAR_ARRAY[DD_PACKET_DATA_LENGTH + 1];
		strncpy(VERSION_STR_CHAR_ARRAY, VERSION_STR, DD_PACKET_DATA_LENGTH);
				
		activeState = '0';
		if (m_NTP.m_NtpActive >= NTP_POLL_ACCEPTABLE_ONLINE_COUNT) 
		{									
			activeState |= DD_PACKET_NETWORKSTATUS_BYTE_NTPACTIVE;
		}
		if (m_PING.m_PingActive >= GATEWAY_PING_ACCEPTABLE_ONLINE_COUNT) 
		{									
			activeState |= DD_PACKET_NETWORKSTATUS_BYTE_PINGACTIVE;
		}
						
		snprintf(version_string, (DD_PACKET_LENGTH + 1), "?%c%c%c%c            %c\r", ip_address_type, VERSION_STR_CHAR_ARRAY[1], VERSION_STR_CHAR_ARRAY[3], VERSION_STR_CHAR_ARRAY[4], activeState);   //PickOut Numbers x from VERSION STR Format VxRxx and Construct the command string first without the check sum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
																				
		uint16_t crc_calc = crc16_ccitt(version_string, (DD_PACKET_DATA_LENGTH + 2));        //calculate the checksum include STX and CMD
				
		snprintf(version_string, (DD_PACKET_LENGTH + 1), "?%c%c%c%c            %c@%04X\r", ip_address_type, VERSION_STR_CHAR_ARRAY[1], VERSION_STR_CHAR_ARRAY[3], VERSION_STR_CHAR_ARRAY[4], activeState, crc_calc);      	//PickOut Numbers x from VERSION STR Format VxRxx and Reconstruct the command string this time with the checksum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
								
		version = version_string;
		
		//std::cout << "VER PACKET OUT [" << version << "]" << std::endl;
		
		return true;		
	}
	
	bool TestLogService::LoadNetworkConfigJSONFile(uint8_t ip_address_type, std::string& ip_address)
	{
	
		try
		{
			std::string json_tag;
			uint8_t activeState = ' ';
			
			// Load json file from current directory
			std::ifstream jsonFile("/home/debian/QRFL_Network_Configuration.json", std::ios::in);
			// JSON object
			nlohmann::json settingsjson;
			// Deserialize json object from text content
			jsonFile >> settingsjson;
			
			switch (ip_address_type)
			{							
			case DD_PACKET_IPADDR_CMD:
				json_tag = "IP_Configuration";				
				break;
			case DD_PACKET_NETMASK_CMD:
				json_tag = "Mask_Configuration";							
				break;
			case DD_PACKET_GATEWAY_CMD:
				json_tag = "Gateway_Configuration";								
				break;
			case DD_PACKET_NTPADDR_CMD:
				json_tag = "NTP_Configuration";							
				break;
			case DD_PACKET_DATETIME_CMD:
				return false;
				break;
			default:
				return false;										
			}
		
			auto value = settingsjson.find(json_tag);
			if (value != settingsjson.end()) 
			{				
				std::string ipaddress_str = value->get<std::string>();
				asio::ip::address_v4 ipaddress = asio::ip::address_v4::from_string(ipaddress_str);				
				char ipaddress_string[DD_PACKET_LENGTH + 2];  //Allow for Null Characer at end plus a spare buye.
				ulong ll;
				ulong ip[4];	
				int i;
				
				ll = ipaddress.to_ulong();
												
				ip[3] = ((ll >> 24) & 0xFF);
				ip[2] = ((ll >> 16) & 0xFF);
				ip[1] = ((ll >> 8) & 0xFF);
				ip[0] = (ll & 0xFF);
												
				activeState = '0';
				if (m_NTP.m_NtpActive >= NTP_POLL_ACCEPTABLE_ONLINE_COUNT) 
				{									
					activeState |= DD_PACKET_NETWORKSTATUS_BYTE_NTPACTIVE;
				}
				if (m_PING.m_PingActive >= GATEWAY_PING_ACCEPTABLE_ONLINE_COUNT) 
				{									
					activeState |= DD_PACKET_NETWORKSTATUS_BYTE_PINGACTIVE;
				}
				
				snprintf(ipaddress_string, (DD_PACKET_LENGTH + 1), "?%c%03d%03d%03d%03d   %c\r", ip_address_type, (int) ip[3], (int) ip[2], (int) ip[1], (int) ip[0], activeState); 	// Construct the command string first without the check sum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
																				
				uint16_t crc_calc = crc16_ccitt(ipaddress_string, (DD_PACKET_DATA_LENGTH + 2));      //calculate the checksum include STX and CMD
				
				snprintf(ipaddress_string, (DD_PACKET_LENGTH + 1), "?%c%03d%03d%03d%03d   %c@%04X\r", ip_address_type, (int) ip[3], (int) ip[2], (int) ip[1], (int) ip[0], activeState, crc_calc);  	// Reconstruct the command string this time with the checksum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
								
				ip_address = ipaddress_string;
												
				//std::cout << "PACKET OUT [" << ip_address << "]" << std::endl;
				
								
			}									
		}
		catch (nlohmann::json::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return false;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return false;
		}    
		
		return true;
	}
	
	bool TestLogService::GetDateTimeQRFLSynch(std::string& datetimeStr)
	{
		uint8_t activeState = ' ';

		try
		{
			
			time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			tm* ptm = std::localtime(&t);
			char datetime_string[DD_PACKET_LENGTH + 2];   //Allow for Null Characer at end plus a spare buye.
			
			activeState = '0';
			if (m_NTP.m_NtpActive >= NTP_POLL_ACCEPTABLE_ONLINE_COUNT) 
			{									
				activeState |= DD_PACKET_NETWORKSTATUS_BYTE_NTPACTIVE;
			}
			if (m_PING.m_PingActive >= GATEWAY_PING_ACCEPTABLE_ONLINE_COUNT) 
			{									
				activeState |= DD_PACKET_NETWORKSTATUS_BYTE_PINGACTIVE;
			}
			
			datetime_string[0] = DD_PACKET_STX;
			datetime_string[1] = DD_PACKET_DATETIME_CMD;
			datetime_string[2] = ((ptm->tm_year - 100) / 10) + 0x30;    //need to subtract 100 as tm_years is num years since 1900
			datetime_string[3] = ((ptm->tm_year - 100) % 10) + 0x30;     //need to subtract 100 as tm_years is num years since 1900	
			datetime_string[4] = ((ptm->tm_mon + 1) / 10) + 0x30;   			//need to add 1 as tm_mon is 0 thru 11 and we need 1 thru 12
			datetime_string[5] = ((ptm->tm_mon + 1) % 10) + 0x30;
			datetime_string[6] = (ptm->tm_mday / 10) + 0x30;
			datetime_string[7] = (ptm->tm_mday % 10) + 0x30;			    			
			datetime_string[8] = (ptm->tm_hour / 10) + 0x30;
			datetime_string[9] = (ptm->tm_hour % 10) + 0x30;
			datetime_string[10] = (ptm->tm_min  / 10) + 0x30;
			datetime_string[11] = (ptm->tm_min % 10) + 0x30;
			datetime_string[12] = (ptm->tm_sec / 10) + 0x30;
			datetime_string[13] = (ptm->tm_sec % 10) + 0x30;
			datetime_string[14] = ' ';
			datetime_string[15] = ' ';
			datetime_string[16] = ' ';
			datetime_string[17] = activeState;
			datetime_string[18] = 0x00;
			
			uint16_t crc_calc = crc16_ccitt(datetime_string, (DD_PACKET_DATA_LENGTH + 2));        //calculate the checksum include STX and CMD
				
			snprintf(&datetime_string[(DD_PACKET_DATA_LENGTH + 2)], (DD_PACKET_CRC_LENGTH + 2), "@%04X\r", crc_calc);     			// Reconstruct the command string this time with the checksum (DD_PACKET_LENGTH + 1) = Allow for Null character at end inserted by snprintf 
																				
			datetimeStr = datetime_string;
												
			//std::cout << "PACKET OUT [" << datetimeStr << "]" << std::endl;
																								
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return false;
		}    
		
		return true;
	}
	
	std::string TestLogService::trim(const std::string& str)
	{
		size_t first = str.find_first_not_of(' ');
		if (first == std::string::npos)
			return "";
		size_t last = str.find_last_not_of(' ');    
		return str.substr(first, (last - first + 1));
	}

	bool TestLogService::isAlphaNumericPacket(const char* packet)
	{
		int i;
		for (i = DD_PACKET_PREAMBLE_LENGTH; i < (DD_PACKET_DATA_LENGTH + DD_PACKET_PREAMBLE_LENGTH); i++)   //Check all Numeric or Spaces or Alpha
			{
				if (!(((packet[i] >= (uint8_t) '0') && (packet[i] <= (uint8_t) '9')) || (packet[i] == (uint8_t) ' ') || ((packet[i] >= (uint8_t) 'A') && (packet[i] <= (uint8_t) 'Z'))))
				{			
					return false;	
				}			
			}
		return true;   
	}

	bool TestLogService::isNumericHexPacket(const char* packet)
	{
		int i;
		for (i = DD_PACKET_PREAMBLE_LENGTH; i < (DD_PACKET_DATA_LENGTH + DD_PACKET_PREAMBLE_LENGTH); i++)   //Check all Numeric or Spaces or Alpha
			{
				if (!(((packet[i] >= (uint8_t) '0') && (packet[i] <= (uint8_t) '9')) || (packet[i] == (uint8_t) ' ') || ((packet[i] >= (uint8_t) 'A') && (packet[i] <= (uint8_t) 'F'))))
				{			
					return false;	
				}			
			}
		return true;   
	}

	bool TestLogService::ProcessSettingsPacket(const char* packet)
	{
		char data[DD_PACKET_DATA_LENGTH + 1];
		std::string temp_str;
    
		switch (packet[1])
		{
		case DD_PACKET_SETTINGS1_STATUS_CMD:
			if (!isAlphaNumericPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			QRFLNameA.assign(data, DD_PACKET_DATA_LENGTH);             
			QRFLNameA = trim(QRFLNameA);
			break;
		case DD_PACKET_SETTINGS2_STATUS_CMD:
			if (!isAlphaNumericPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			QRFLNameB.assign(data, DD_PACKET_DATA_LENGTH);             
			QRFLNameB = trim(QRFLNameB);                                       
			break;
		case DD_PACKET_SETTINGS3_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%08X%02X%02X%04X", &QRFLConfiguration, &QRFLUnitNumber, &QRFLMode, &QRFLActiveUnitList); 
			break;
		case DD_PACKET_SETTINGS4_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%04X%04X%02X%02X%04X", &QRFLRTUT0, &QRFLILogThreshold, &QRFLRTUErrorLimit, &QRFLModemLimit, &QRFLVTDetectThreshold); 
			break;								
		case DD_PACKET_SETTINGS5_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%04X%04X%02X%02X%04X", &QRFLITDetectThreshold, &QRFLSamplingStopDelay, &QRFLSleepTime, &QRFLCLRWaitTime, &QRFLQTime); 
			break;								            
		case DD_PACKET_SETTINGS6_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%04X%04X%02X%02X0000", &QRFLISRTime, &QRFLLANLatencyMilliSecs, &QRFLRxGain, &QRFLTxGain); 
			break;
            
		case DD_PACKET_SETTINGS7_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%03d%03d%03d%03d0000", &QRFLIPAddress[0], &QRFLIPAddress[1], &QRFLIPAddress[2], &QRFLIPAddress[3]); 
			break;
		case DD_PACKET_SETTINGS8_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%03d%03d%03d%03d0000", &QRFLNetMaskAddress[0], &QRFLNetMaskAddress[1], &QRFLNetMaskAddress[2], &QRFLNetMaskAddress[3]); 
			break;
		case DD_PACKET_SETTINGS9_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%03d%03d%03d%03d0000", &QRFLGatewayAddress[0], &QRFLGatewayAddress[1], &QRFLGatewayAddress[2], &QRFLGatewayAddress[3]); 
			break;									
		case DD_PACKET_SETTINGS10_STATUS_CMD:
			if (!isNumericHexPacket(packet)) return false;
			std::strncpy(data, &packet[DD_PACKET_PREAMBLE_LENGTH], DD_PACKET_DATA_LENGTH + 1);
			data[DD_PACKET_DATA_LENGTH] = 0x00;
			sscanf(data, "%03d%03d%03d%03d0000", &QRFLNTPAddress[0], &QRFLNTPAddress[1], &QRFLNTPAddress[2], &QRFLNTPAddress[3]); 															
			break;
		}
		return true;
	}
	
	
	bool TestLogService::CreateParameterSettingsJSONFile()
	{		
		std::string json_tag;
		std::string dbm[] = { "-10.5", "- 9.0", "- 7.5", "- 6.0", "- 4.5", "- 3.0", "- 1.5", "  0.0" };
		bool isFileSame = true;
		bool isValidFile = true;
		int i;
			
		nlohmann::json settingsjson =
		{
			{	
				"parameters0",
				{				
					{			
						{"qrfl_site_name", "DEFAULT"},
						{"qrfl_unit_id", "0"},
						{"qrfl_active_units", "1"},							
						{"qrfl_ip_address", "0.0.0.0"},
						{"qrfl_netmask_address", "0.0.0.0"},
						{"qrfl_gateway_address", "0.0.0.0"},
						{"qrfl_ntp_address", "0.0.0.0"}													
					}
				}
			},
			{	
				"parameters1",
				{				
					{			
						{"dc_filter", "on"},
						{"ext_rtu", "on"},
						{"mode", "v23"},
						{"v23", "off"},
						{"log", "on"},
						{"vt_detect", "on"},
						{"it_detect", "off"}
					}
				}
		},
		{			
			"parameters2", 
			{			
				{
					{"rtu_time_out", "05"},
					{"amps_log_thld", "1200"},
					{"rtu_limit", "4"},
					{"mdm_limit", "3"},
					{"vt_thld", "100"},
					{"it_thld", "0500"},
					{"bs_length", "096"},
					{"sleep_time", "10"},
					{"clr_wait", "10"},
					{"q_time", "60"},
					{"isr_time", "30"}
				}
			}
		},
		{			
			"parameters3", 
			{			
				{
					{"rx_gain", "-9"},
					{"tx_gain", "0.0"}
				}
			}
		}
		};
		
		if (StdOutDebug) 
		{
					
			std::cout << "QRFLNAMEA '" << QRFLNameA << "'\n";
			std::cout << "QRFLNAMEB '" << QRFLNameB << "'\n";  
			std::cout << "QRFLConfiguration '" << QRFLConfiguration << "'\n";
			std::cout << "QRFLUnitNumber '" << QRFLUnitNumber << "'\n";
			std::cout << "QRFLMode '" << QRFLMode << "'\n";
			std::cout << "QRFLActiveUnitList '" << QRFLActiveUnitList << "'\n";
        
			std::cout << "QRFLRTUT0 '" << QRFLRTUT0 << "'\n";
			std::cout << "QRFLILogThreshold '" << QRFLILogThreshold << "'\n";
			std::cout << "QRFLRTUErrorLimit '" << QRFLRTUErrorLimit << "'\n";
			std::cout << "QRFLModemLimit '" << QRFLModemLimit << "'\n";
			std::cout << "QRFLVTDetectThreshold '" << QRFLVTDetectThreshold << "'\n";
    
			std::cout << "QRFLITDetectThreshold '" << QRFLITDetectThreshold << "'\n";
			std::cout << "QRFLSamplingStopDelay '" << QRFLSamplingStopDelay << "'\n";
			std::cout << "QRFLSleepTime '" << QRFLSleepTime << "'\n";
			std::cout << "QRFLCLRWaitTime '" << QRFLCLRWaitTime << "'\n";
			std::cout << "QRFLQTime '" << QRFLQTime << "'\n";
    
			std::cout << "QRFLISRTime '" << QRFLISRTime << "'\n";
			std::cout << "QRFLLANLatencyMilliSecs '" << QRFLLANLatencyMilliSecs << "'\n";
			std::cout << "QRFLRxGain '" << QRFLRxGain << "'\n";
			std::cout << "QRFLTxGain '" << QRFLTxGain << "'\n";
    
			std::cout << "QRFLIPAddress '" << QRFLIPAddress[0] << "." << QRFLIPAddress[1] << "." << QRFLIPAddress[2] << "." << QRFLIPAddress[3] << "'\n";
			std::cout << "QRFLNetMaskAddress '" << QRFLNetMaskAddress[0] << "." << QRFLNetMaskAddress[1] << "." << QRFLNetMaskAddress[2] << "." << QRFLNetMaskAddress[3] << "'\n";
			std::cout << "QRFLGatewayAddress '" << QRFLGatewayAddress[0] << "." << QRFLGatewayAddress[1] << "." << QRFLGatewayAddress[2] << "." << QRFLGatewayAddress[3] << "'\n";
			std::cout << "QRFLNTPAddress '" << QRFLNTPAddress[0] << "." << QRFLNTPAddress[1] << "." << QRFLNTPAddress[2] << "." << QRFLNTPAddress[3] << "'\n";
		}
		
		try
		{
			
			for (i = 0; i < MAX_NUM_SETTING_COMMANDS; i++)
			{
				isValidFile &= receivedValidSettingsData[i];
				receivedValidSettingsData[i] = false;
				isFileSame &= (previousSettingsReplyString[i] == currentSettingsReplyString[i]);
				previousSettingsReplyString[i] = currentSettingsReplyString[i];				
			}
			
			if (!isValidFile) return false;   //Do not write and incompletly recevied file
			if (isFileSame) return true;      //Do not write a file if it has not changed since last time.
									
			settingsjson["parameters0"][0]["qrfl_site_name"] = QRFLNameA + QRFLNameB;	
			settingsjson["parameters0"][0]["qrfl_active_units"] = std::to_string(QRFLActiveUnitList);			
			settingsjson["parameters0"][0]["qrfl_unit_id"] = std::to_string(QRFLUnitNumber);
			settingsjson["parameters0"][0]["qrfl_ip_address"] = std::to_string(QRFLIPAddress[0]) + "." + std::to_string(QRFLIPAddress[1]) + "." + std::to_string(QRFLIPAddress[2]) + "." + std::to_string(QRFLIPAddress[3]);
			settingsjson["parameters0"][0]["qrfl_netmask_address"] = std::to_string(QRFLNetMaskAddress[0]) + "." + std::to_string(QRFLNetMaskAddress[1]) + "." + std::to_string(QRFLNetMaskAddress[2]) + "." + std::to_string(QRFLNetMaskAddress[3]);
			settingsjson["parameters0"][0]["qrfl_gateway_address"] = std::to_string(QRFLGatewayAddress[0]) + "." + std::to_string(QRFLGatewayAddress[1]) + "." + std::to_string(QRFLGatewayAddress[2]) + "." + std::to_string(QRFLGatewayAddress[3]);
			settingsjson["parameters0"][0]["qrfl_ntp_address"] = std::to_string(QRFLNTPAddress[0]) + "." + std::to_string(QRFLNTPAddress[1]) + "." + std::to_string(QRFLNTPAddress[2]) + "." + std::to_string(QRFLNTPAddress[3]);
							
			if (QRFLConfiguration & CONFIG_DC_FILTER) 
			{
				settingsjson["parameters1"][0]["dc_filter"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["dc_filter"] = "off";		
			}
			
			if (QRFLConfiguration & MK3_RTU_FORMAT) 
			{
				settingsjson["parameters1"][0]["ext_rtu"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["ext_rtu"] = "off";		
			}
			
			if (QRFLConfiguration & V23_ONLINE) 
			{
				settingsjson["parameters1"][0]["v23"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["v23"] = "off";		
			}
			
			if (QRFLConfiguration & LOG_ONLINE) 
			{
				settingsjson["parameters1"][0]["log"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["log"] = "off";		
			}
			
			if (QRFLConfiguration & VT_ENABLED) 
			{
				settingsjson["parameters1"][0]["vt_detect"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["vt_detect"] = "off";		
			}
			
			if (QRFLConfiguration & IT_ENABLED) 
			{
				settingsjson["parameters1"][0]["it_detect"] = "on";		
			}
			else
			{
				settingsjson["parameters1"][0]["it_detect"] = "off";		
			}
					
			if (QRFLMode == QRFL_MODE_V23)
			{
				settingsjson["parameters1"][0]["mode"] = "V23";				
			}
			else
			{
				settingsjson["parameters1"][0]["mode"] = "V23+LAN";					
			}
																
			settingsjson["parameters2"][0]["rtu_time_out"] = std::to_string(QRFLRTUT0);			
			settingsjson["parameters2"][0]["amps_log_thld"] = std::to_string(QRFLILogThreshold);
			settingsjson["parameters2"][0]["rtu_limit"] = std::to_string(QRFLRTUErrorLimit);
			settingsjson["parameters2"][0]["mdm_limit"] = std::to_string(QRFLModemLimit);
			settingsjson["parameters2"][0]["vt_thld"] = std::to_string(QRFLVTDetectThreshold);
			settingsjson["parameters2"][0]["it_thld"] = std::to_string(QRFLITDetectThreshold);
			settingsjson["parameters2"][0]["bs_length"] = std::to_string(QRFLSamplingStopDelay);
			settingsjson["parameters2"][0]["sleep_time"] = std::to_string(QRFLSleepTime);
			settingsjson["parameters2"][0]["clr_wait"] = std::to_string(QRFLCLRWaitTime);			
			settingsjson["parameters2"][0]["q_time"] = std::to_string(QRFLQTime);
			settingsjson["parameters2"][0]["isr_time"] = std::to_string(QRFLISRTime);
			
			
			if (QRFLRxGain < sizeof(dbm))
			{
				settingsjson["parameters3"][0]["rx_gain"] = dbm[QRFLRxGain];
			}	
			else
			{
				settingsjson["parameters3"][0]["rx_gain"] = "0.0";
			}
			
			if (QRFLTxGain < sizeof(dbm))
			{
				settingsjson["parameters3"][0]["tx_gain"] = dbm[QRFLTxGain];
			}	
			else
			{
				settingsjson["parameters3"][0]["tx_gain"] = "0.0";
			}
																											
			std::cout << "CREATE PARAMETER SETTINGS FILE" << std::endl;						
			std::ofstream outfile("/home/debian/bridge-app/parameters.json", std::ios::out);
			outfile << settingsjson;
			outfile.flush();
			outfile.close();
																								
		}
		catch (nlohmann::json::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			return false;
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
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

		
	bool TestLogService::BlinkLED()
	{
		FILE *export_file = NULL;  //declare pointers
		FILE *IO_direction = NULL;
		FILE *IO_value = NULL;
		char str1[] = "0";
		char str2[] = "1";
		char str3[] = "out";
		char str4[] = "in";
		char str[] = "44";

		//this part here exports gpio44
		export_file = fopen("/sys/class/gpio/export", "w");
		fwrite(str, 1, sizeof(str), export_file);
		fclose(export_file);

		//this part here sets Pin as Output
		IO_direction = fopen("/sys/class/gpio/gpio44/direction", "w");
		fwrite(str3, 1, sizeof(str3), IO_direction);  //set the pin to Output
		fclose(IO_direction);

		//Sets Pin to High
		IO_value = fopen("/sys/class/gpio/gpio44/value", "w");
		fwrite(str2, 1, sizeof(str2), IO_value);  //set the pin to HIGH
		fclose(IO_value);
		
		usleep(250000);  //delay for 0.25 seconds

		//Sets Pin to Low  
		IO_value = fopen("/sys/class/gpio/gpio44/value", "w");
		fwrite(str1, 1, sizeof(str1), IO_value);  //set the pin to LOW
		fclose(IO_value);
	}



	
} /* namespace aurizon */
