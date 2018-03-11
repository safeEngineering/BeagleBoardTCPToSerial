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
#include "NTP.hpp"
#include "PING.hpp"
#include <chrono>
#include <memory>
#include <string>

//Packet Format ?C123456789012   N@XXXX\r
#define DD_PACKET_CRC_LENGTH 5
#define DD_PACKET_DATA_LENGTH 16
#define DD_PACKET_STX '?'
#define DD_PACKET_STX_STR "?"
#define DD_PACKET_ETX_STR "\r"
#define DD_PACKET_PREAMBLE_LENGTH 2   // ?C
#define DD_PACKET_CRC_TAG '@'
#define DD_PACKET_REQ_NETWORK_STATUS 'a'
#define DD_PACKET_IPADDR_CMD 'A'
#define DD_PACKET_NETMASK_CMD 'B'
#define DD_PACKET_GATEWAY_CMD 'C'
#define DD_PACKET_NTPADDR_CMD 'D'
#define DD_PACKET_DATETIME_CMD 'E'
#define DD_PACKET_VERSION_CMD 'F'
#define DD_PACKET_TRIP_START_CHANGE_CMD 'G'
#define DD_PACKET_ALARMS_CHANGE_CMD 'H'
#define DD_PACKET_TRIP_START_STATUS_CMD 'g'
#define DD_PACKET_ALARMS_STATUS_CMD 'h'
#define DD_PACKET_SETTINGS_REQ_CMD 'I'
#define DD_PACKET_SETTINGS1_STATUS_CMD 'i'
#define DD_PACKET_SETTINGS2_STATUS_CMD 'j'
#define DD_PACKET_SETTINGS3_STATUS_CMD 'k'
#define DD_PACKET_SETTINGS4_STATUS_CMD 'l'
#define DD_PACKET_SETTINGS5_STATUS_CMD 'm'
#define DD_PACKET_SETTINGS6_STATUS_CMD 'n'
#define DD_PACKET_SETTINGS7_STATUS_CMD 'o'
#define DD_PACKET_SETTINGS8_STATUS_CMD 'p'
#define DD_PACKET_SETTINGS9_STATUS_CMD 'q'
#define DD_PACKET_SETTINGS10_STATUS_CMD 'r'
#define DD_PACKET_RESET_STATUS_CMD 's'
#define DD_PACKET_REQ_RESET_CMD 'S'
#define MAX_NUM_SETTING_COMMANDS 10
#define DD_PACKET_NETWORKSTATUS_BYTE_POS (DD_PACKET_DATA_LENGTH-1) 
#define DD_PACKET_NETWORKSTATUS_BYTE_PINGACTIVE 0x01
#define DD_PACKET_NETWORKSTATUS_BYTE_NTPACTIVE 0x02
#define DD_PACKET_LENGTH (DD_PACKET_DATA_LENGTH+3+DD_PACKET_CRC_LENGTH)   //STX, CMD, DATA, CRC, ETX  

#define TIME_SYNC_TOLERANCE_SECS 5
#define TIME_SYNC_REPEAT_LOCKOUT  5

namespace aurizon
{

class NodeServer;

class TestLogService : public std::enable_shared_from_this<TestLogService>
{
private:
    TestLogService(asio::io_service &ioService, const std::string &strPort,
		uint32_t nBaud, bool consoleDebug);

public:
    TestLogService(const TestLogService&) = delete;
    virtual ~TestLogService();

    TestLogService& operator=(const TestLogService&) = delete;

    void start();
    void stop();

    static std::shared_ptr<TestLogService> Instance(asio::io_service &ioService,
		const std::string &strPort,
		uint32_t nBaud,
		bool consoleDebug);

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
	
	std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
	
	void SimulateDatatoLog(int counter);
	bool ParseDebugLogText(std::string m_strBuffer, std::string delimiterStart, std::string delimiterEnd, bool isPacketFormat);
	bool ParseCommand(std::string str);
	bool ParseText(std::string str);
	std::string GetTimeAsString(std::string formatString, time_t theTime);

    void addLog(const std::string& str);
	
	bool SendBasicCommand(uint8_t command_type, std::string& command);
	void ReadNTPandGatewayIPAddresses();
	void CreateNetworkSettingsJSONFile();
	void UpdateNetworkSettingsJSONFile(uint8_t ip_address_type, std::string ip_address);
	bool LoadNetworkConfigJSONFile(uint8_t ip_address_type, std::string &ip_address);
	bool LoadVersion(uint8_t ip_address_type, std::string& version);
	bool GetDateTimeQRFLSynch(std::string& datetimeStr);
	bool BlinkLED();
	bool isAlphaNumericPacket(const char* packet);
	bool isNumericHexPacket(const char* packet);
	bool ProcessSettingsPacket(const char* packet);
	std::string trim(const std::string& str);
	bool CreateParameterSettingsJSONFile();

    static const uint32_t MAX_BUFFER_SIZE = 4096;

    asio::io_service& m_ioService;
    std::shared_ptr<asio::serial_port> m_ptrSerialPort;
    Timer<std::chrono::steady_clock> m_timer;
	Timer<std::chrono::steady_clock> r_timer;
    const std::string m_strPort;
    const uint32_t m_nBaud;

    uint8_t m_aReadBuffer[MAX_BUFFER_SIZE];
    std::string m_strBuffer;
	
	time_t QRFLTime = 0;
	
    std::size_t m_nSearchIndex = 0;

    std::chrono::system_clock::time_point m_logTime;;

    int32_t m_nReadID = 0;

    bool m_bIsRunning = false;
    bool m_bRestarting = false;
	
	int16_t sequenceCounter = 0;
	
	uint8_t validNetworkSettingsRxState = 0;
	
	asio::ip::address_v4 IP;
	asio::ip::address_v4 GATEWAY;
	asio::ip::address_v4 NETMASK;
	asio::ip::address_v4 NTPSERVER;
			
	int16_t oldDetailsMatchCounter = 0;
	std::string oldIPAddress = "";
	std::string oldGatewayAddress = "";
	std::string oldNetmaskAddress = "";
	std::string oldNTPServerAddress = "";
	
	double QRFLTimeDifference = 0;
	double QRFLTimeDifferenceBlankOutCounter = 0;
	
	bool StdOutDebug = false;
	
	int16_t debugCounter = 1;		//Temporary Variable to make command sequences happen for testing
	
	std::string alarmLogDataString = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
	std::string tripLogDataString = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
	std::string prevAlarmLogDataString = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
	std::string prevTripLogDataString = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
	
	//QRFL Settings
	std::string QRFLNameA;
	std::string QRFLNameB;
	unsigned int QRFLConfiguration;
	unsigned int QRFLUnitNumber;
	unsigned int QRFLMode;
	unsigned int QRFLActiveUnitList;

	unsigned int QRFLRTUT0;
	unsigned int QRFLILogThreshold;
	unsigned int QRFLRTUErrorLimit;
	unsigned int QRFLModemLimit;
	unsigned int QRFLVTDetectThreshold;

	unsigned int QRFLITDetectThreshold;
	unsigned int QRFLSamplingStopDelay;
	unsigned int QRFLSleepTime;
	unsigned int QRFLCLRWaitTime;
	unsigned int QRFLQTime;

	unsigned int QRFLISRTime;
	unsigned int QRFLLANLatencyMilliSecs;
	unsigned int QRFLRxGain;
	unsigned int QRFLTxGain;

	unsigned int QRFLIPAddress[4];
	unsigned int QRFLNetMaskAddress[4];
	unsigned int QRFLGatewayAddress[4];
	unsigned int QRFLNTPAddress[4];
	//End QRFL Settings
	
	
	//QRFL Settings File Generation Management 
	bool receivedValidSettingsData[MAX_NUM_SETTING_COMMANDS];
	std::string previousSettingsReplyString[MAX_NUM_SETTING_COMMANDS];
	std::string currentSettingsReplyString[MAX_NUM_SETTING_COMMANDS];
	//End QRFL Settings File Generation Management 
	
	uint16_t crc16_ccitt(const char *buf, int len);
	
	//Ntp Client
	SafeEngineering::Comm::NTP m_NTP;
	
	//Ping Client
	SafeEngineering::Comm::PING m_PING;
	
    const uint16_t crc16tab[256] = {
		0x0000,
		 0x1021,
		 0x2042,
		 0x3063,
		 0x4084,
		 0x50a5,
		 0x60c6,
		 0x70e7,
		0x8108,
		 0x9129,
		 0xa14a,
		 0xb16b,
		 0xc18c,
		 0xd1ad,
		 0xe1ce,
		 0xf1ef,
		0x1231, 
		0x0210,
		 0x3273,
		 0x2252,
		 0x52b5,
		 0x4294,
		 0x72f7, 
		0x62d6,
		0x9339,
		 0x8318,
		 0xb37b,
		 0xa35a,
		 0xd3bd,
		 0xc39c,
		 0xf3ff, 
		0xe3de,
		0x2462,
		 0x3443,
		 0x0420,
		 0x1401,
		 0x64e6,
		 0x74c7, 
		0x44a4, 
		0x5485,
		0xa56a,
		 0xb54b,
		 0x8528,
		 0x9509,
		 0xe5ee, 
		0xf5cf, 
		0xc5ac, 
		0xd58d,
		0x3653,
		 0x2672,
		 0x1611,
		 0x0630,
		 0x76d7, 
		0x66f6, 
		0x5695, 
		0x46b4,
		0xb75b,
		 0xa77a,
		 0x9719,
		 0x8738, 
		0xf7df,
		 0xe7fe, 
		0xd79d, 
		0xc7bc,
		0x48c4,
		 0x58e5,
		 0x6886,
		 0x78a7, 
		0x0840,
		 0x1861,
		 0x2802, 
		0x3823,
		0xc9cc,
		 0xd9ed,
		 0xe98e, 
		0xf9af,
		 0x8948,
		 0x9969,
		 0xa90a, 
		0xb92b,
		0x5af5,
		 0x4ad4, 
		0x7ab7,
		 0x6a96,
		 0x1a71,
		 0x0a50, 
		0x3a33, 
		0x2a12,
		0xdbfd,
		 0xcbdc, 
		0xfbbf,
		 0xeb9e,
		 0x9b79,
		 0x8b58, 
		0xbb3b, 
		0xab1a,
		0x6ca6,
		 0x7c87,
		 0x4ce4,
		 0x5cc5,
		 0x2c22,
		 0x3c03,
		 0x0c60, 
		0x1c41,
		0xedae, 
		0xfd8f, 
		0xcdec,
		 0xddcd,
		 0xad2a,
		 0xbd0b, 
		0x8d68, 
		0x9d49,
		0x7e97, 
		0x6eb6,
		 0x5ed5,
		 0x4ef4,
		 0x3e13,
		 0x2e32,
		 0x1e51, 
		0x0e70,
		0xff9f,
		 0xefbe,
		 0xdfdd,
		 0xcffc,
		 0xbf1b,
		 0xaf3a,
		 0x9f59,
		 0x8f78,
		0x9188,
		 0x81a9,
		 0xb1ca,
		 0xa1eb,
		 0xd10c,
		 0xc12d,
		 0xf14e, 
		0xe16f,
		0x1080, 
		0x00a1,
		 0x30c2,
		 0x20e3,
		 0x5004,
		 0x4025,
		 0x7046, 
		0x6067,
		0x83b9, 
		0x9398,
		 0xa3fb,
		 0xb3da,
		 0xc33d,
		 0xd31c,
		 0xe37f, 
		0xf35e,
		0x02b1,
		 0x1290, 
		0x22f3, 
		0x32d2,
		 0x4235,
		 0x5214, 
		0x6277, 
		0x7256,
		0xb5ea,
		 0xa5cb,
		 0x95a8, 
		0x8589,
		 0xf56e,
		 0xe54f,
		 0xd52c, 
		0xc50d,
		0x34e2,
		 0x24c3,
		 0x14a0, 
		0x0481,
		 0x7466,
		 0x6447,
		 0x5424, 
		0x4405,
		0xa7db,
		 0xb7fa,
		 0x8799,
		 0x97b8, 
		0xe75f,
		 0xf77e, 
		0xc71d, 
		0xd73c,
		0x26d3,
		 0x36f2,
		 0x0691,
		 0x16b0, 
		0x6657, 
		0x7676, 
		0x4615, 
		0x5634,
		0xd94c,
		 0xc96d,
		 0xf90e,
		 0xe92f,
		 0x99c8, 
		0x89e9,
		 0xb98a, 
		0xa9ab,
		0x5844,
		 0x4865,
		 0x7806,
		 0x6827,
		 0x18c0, 
		0x08e1,
		 0x3882, 
		0x28a3,
		0xcb7d,
		 0xdb5c,
		 0xeb3f,
		 0xfb1e,
		 0x8bf9,
		 0x9bd8,
		 0xabbb, 
		0xbb9a,
		0x4a75,
		 0x5a54,
		 0x6a37,
		 0x7a16,
		 0x0af1,
		 0x1ad0,
		 0x2ab3, 
		0x3a92,
		0xfd2e,
		 0xed0f,
		 0xdd6c,
		 0xcd4d,
		 0xbdaa,
		 0xad8b, 
		0x9de8, 
		0x8dc9,
		0x7c26,
		 0x6c07,
		 0x5c64,
		 0x4c45,
		 0x3ca2,
		 0x2c83, 
		0x1ce0,
		 0x0cc1,
		0xef1f,
		 0xff3e,
		 0xcf5d,
		 0xdf7c,
		 0xaf9b,
		 0xbfba,
		 0x8fd9, 
		0x9ff8,
		0x6e17, 
		0x7e36,
		 0x4e55,
		 0x5e74,
		 0x2e93,
		 0x3eb2,
		 0x0ed1,
		 0x1ef0
	};
	
	
		
};

} /* namespace aurizon */

#endif /* __TESTLOGSERVICE_H_ */
