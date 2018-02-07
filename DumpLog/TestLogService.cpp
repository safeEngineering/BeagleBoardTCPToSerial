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

//Packet Format ?C123456789012   N@XXXX\r
#define DD_PACKET_CRC_LENGTH 5
#define DD_PACKET_DATA_LENGTH 16
#define DD_PACKET_STX '?'
#define DD_PACKET_ETX "\r"
#define DD_PACKET_PREAMBLE_LENGTH 2   // ?C
#define DD_PACKET_CRC_TAG '@'
#define DD_PACKET_IPADDR_CMD 'A'
#define DD_PACKET_NETMASK_CMD 'B'
#define DD_PACKET_GATEWAY_CMD 'C'
#define DD_PACKET_NTPADDR_CMD 'D'
#define DD_PACKET_DATETIME_CMD 'E'
#define DD_PACKET_VERSION_CMD 'F'
#define DD_PACKET_NETWORKSTATUS_BYTE_POS (DD_PACKET_DATA_LENGTH-1) 
#define DD_PACKET_NETWORKSTATUS_BYTE_PINGACTIVE 0x01
#define DD_PACKET_NETWORKSTATUS_BYTE_NTPACTIVE 0x02
#define DD_PACKET_LENGTH (DD_PACKET_DATA_LENGTH+3+DD_PACKET_CRC_LENGTH)   //STX, CMD, DATA, CRC, ETX  

#define TIME_SYNC_TOLERANCE_SECS 5
#define TIME_SYNC_REPEAT_LOCKOUT  5


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
        m_ioService(ioService), m_timer(ioService), r_timer(ioService), m_strPort(strPort), m_nBaud(nBaud), m_NTP(ioService, true, "8.8.8.8"), m_PING(ioService, true, "8.8.8.8")
{
		sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
		m_NTP.m_NtpActive = 0;
		m_PING.m_PingActive = 0;			
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
				
				std::cout << "GATEWAY IP ADDRESS " << ip_address << std::endl;
			
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
				
				std::cout << "NTP IP ADDRESS " << ip_address << std::endl;
			
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
		
	if ((QRFLTimeDifference > TIME_SYNC_TOLERANCE_SECS) && (QRFLTimeDifferenceBlankOutCounter == 0) && (m_NTP.m_NtpActive >= NTP_POLL_ACCEPTABLE_ONLINE_COUNT))  //If time out of synch and NTP Server is active
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
			ReadNTPandGatewayIPAddresses();						//Update the IP and NTP here as good as any place
			sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
			break;
		default:
			inputStr = "";
			sequenceCounter = (int16_t) DD_PACKET_IPADDR_CMD;
			break;	
		}
	}
		
	if (inputStr != "")
	{			
		auto self = shared_from_this();
		std::cout << "Out Str - " << inputStr <<  std::endl;  
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
	time_t QRFLTime;
			
						
	char IPAddressString[DD_PACKET_DATA_LENGTH + 1];
		
	if (str.length() != (DD_PACKET_LENGTH-1))  // Leave Out ETX in length comparison
	{
		return false;
	}
			
	rxbuffer = str.c_str();
	
	if (rxbuffer[0] != DD_PACKET_STX) return false;   //Check for STX
	
	if (rxbuffer[(DD_PACKET_DATA_LENGTH + 2)] != DD_PACKET_CRC_TAG) return false;   // Check for CRC
								
	for (i = 2; i < (DD_PACKET_DATA_LENGTH + 2); i++)   //Check all Numeric or Spaces
	{
		if (!( ((rxbuffer[i] >= (uint8_t) '0') && (rxbuffer[i] <= (uint8_t) '9')) || (rxbuffer[i] == (uint8_t) ' ')) )
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
			
	if ((rxbuffer[1] != DD_PACKET_DATETIME_CMD))   //i.e This is an IP Address Command
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
				
				std::cout << "QRFL TIME - " << date << "-" << month << "-" << year << " " << hr << ":" << min << ":" << sec <<  "{" << QRFLTimeDifference << "}" << std::endl;						
												
			}
			else
			{
				QRFLTimeDifference = 0;
			}
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
