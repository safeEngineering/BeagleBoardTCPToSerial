#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <iostream>
#include <iomanip>
#include <deque>
#include <string>
#include <cmath>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <asio.hpp>

#define SERIAL_BUFFER_LENGTH            40
#define SERIAL_PACKET_LENGTH            10

#define START_OF_PACKET                 0x02
#define END_OF_PACKET                   0x03

#define GPIO_PIN_READ_PERIOD            250         // ms
#define DEBOUNCE_SAMPLES                4
#define HIGH_SAMPLES_MASK               (pow(2, DEBOUNCE_SAMPLES) - 1)

#define HEADER                          P9
#define PIN                             27

#include "bbb.h"

#include "SerialPacket.hpp"
#include "Utils.hpp"

#include "spdlog/spdlog.h"
#include "DailyFileSink.hpp"

//#define SIM_MASTER_MODE 1

//#define SIM_SLAVE_MODE 1

namespace SafeEngineering
{
    namespace Comm
    {
        class Serial
        {
        public:
	        Serial(asio::io_service& io, bool consoleDebug)
		        : m_serialPort(io) 
		        , StdOutDebug(consoleDebug)
                , m_signalSet(io)
                , m_GPIOReadTimer(io)
            {
                m_signalSet.add(SIGINT);
                m_signalSet.add(SIGTERM);
            }
            
            ~Serial()
            {
            }
            
            // The event will signal whenever receiving the data packet from external serial device
            boost::signals2::signal<void(uint8_t*, uint8_t)> m_DataReceived;
            
            void OpenSerial()
            {
                try
                {
                    if(m_serialPort.is_open() == true)
                    {
                        // First. Close COM port because it was opened before
                        CloseSerial();
                    }
                    
                    // Initialize the termination signals
                    m_signalSet.async_wait(std::bind(&Serial::HandleSignal, this, std::placeholders::_1, std::placeholders::_2));
                    
                    // Open the COM port
                    m_serialPort.open("/dev/ttyS1");  // Name of COM/UART port ("COMx" on Windows or "/dev/ttySx" on Linux)
                    // Set baud rate
                    asio::serial_port_base::baud_rate baud(38400);
                    m_serialPort.set_option(baud);
                    // Set parity
                    asio::serial_port_base::parity parity(asio::serial_port_base::parity::even);  //AE ETHERNET COMMS
                    m_serialPort.set_option(parity);
                    // Set flow control
                    asio::serial_port_base::flow_control flowCtrl(asio::serial_port_base::flow_control::none);
                    m_serialPort.set_option(flowCtrl);
                    // Set data bits
                    asio::serial_port_base::character_size dataBits(8);
                    m_serialPort.set_option(dataBits);
                    // Set stop bits
                    asio::serial_port_base::stop_bits stopBits(asio::serial_port_base::stop_bits::one);
                    m_serialPort.set_option(stopBits);
                    
                    // Clear data structure
                    memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                    m_packetLen = 0;
                    
                    // Setup asynchonous read operation
                    StartAsyncRead();
                    
                    // Initialize GPIO_PIN_NAME pin as input pin
                    InitializeGPIOPin();
                }
                catch(std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }
            
            void CloseSerial()
            {
                m_serialPort.close();
            }

#ifdef SIM_SLAVE_MODE	        
            bool SendPacket(uint8_t* pPacket, size_t len)
#else
	        bool SendPacket(const uint8_t* pPacket, size_t len)
#endif	
            {
                if(len != SERIAL_PACKET_LENGTH)
                    return false;
                if(m_serialPort.is_open() == false)
                    return false;

#ifdef SIM_SLAVE_MODE
				   	            
	            if (pPacket[1] == 0xF1)
	            {
		            pPacket[0] = 0x02;
		            pPacket[1] = 0x6F;
		            pPacket[2] = 0x51;
		            pPacket[3] = 0x36;
		            pPacket[4] = 0x30;
		            pPacket[5] = 0x30;
		            pPacket[6] = 0x3A;
		            pPacket[7] = 0x3D;
		            pPacket[8] = 0x00;
		            pPacket[9] = 0x03;
		            
		            std::cout  << "Sending bytes: " << Utils::ConvertToHex(pPacket, (int)len) << " to UART port" << std::endl;
	            }
	            else if (pPacket[1] == 0xC1)
	            {		            
		            pPacket[0] = 0x02;
		            pPacket[1] = 0x6F;
		            pPacket[2] = 0x41;
		            pPacket[3] = 0x31;
		            pPacket[4] = 0x34;
		            pPacket[5] = 0x38;
		            pPacket[6] = 0x3C;
		            pPacket[7] = 0x00;
		            pPacket[8] = 0x00;
		            pPacket[9] = 0x03;
		            
		            std::cout  << "Sending bytes: " << Utils::ConvertToHex(pPacket, (int)len) << " to UART port" << std::endl;
	            }
	            else
	            {		            
		            pPacket[0] = 0x00;
		            pPacket[1] = 0x00;
		            pPacket[2] = 0x00;
		            pPacket[3] = 0x00;
		            pPacket[4] = 0x00;
		            pPacket[5] = 0x00;
		            pPacket[6] = 0x00;
		            pPacket[7] = 0x00;
		            pPacket[8] = 0x00;
		            pPacket[9] = 0x00;
		            
		        }
	            
#endif	            
	            // Construct new packet for the queue
                SerialPacket packet(pPacket, len);
                
                // Keep in mind. Calling to empty() will been ensured as atomic operation!!!
                bool write_in_progress = !m_writePackets.empty();                
                m_writePackets.push_back(packet);
                if(!write_in_progress)
                {
                    if (StdOutDebug) std::cout  << "Sending bytes: " << Utils::ConvertToHex(pPacket, (int)len) << " to UART port" << std::endl;
	                
	                //Log Outgoing Serial Data
	                char character_data[SERIAL_PACKET_LENGTH];
	                for (int ii = 0; ii < (int)len; ii++)
	                {
		                character_data[ii] = (char) pPacket[ii];
	                }
	                std::string message(character_data, (size_t) len);
			
	                std::string hexMessage = Utils::string_to_hex(message);
					
	                spdlog::get("E23DataLog")->info() << "OUT:" << hexMessage;
                    //spdlog::get("E23DataLog")->info("OUT: {}", hexMessage);
                    
                    // Send this packet to serial port now
                    asio::async_write(m_serialPort, asio::buffer(m_writePackets.front().Data(), m_writePackets.front().Length()), 
                        boost::bind(&Serial::HandleWrite, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                }
                else
                {
                    if (StdOutDebug) std::cout  << "Push the packet to the queue" << std::endl;
                }
                
                return true;
            }
            
        private:        
            void StartAsyncRead()
            {
                // Clear content of buffer
                m_serialPort.async_read_some(asio::buffer(m_buffer), boost::bind(&Serial::ReadHandler, 
                    this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                if (StdOutDebug) std::cout  << "Started asynchonous UART read operation" << std::endl;
            }
            
            void ReadHandler(const asio::error_code& err, std::size_t bytes)
            {
                if(!err)
                {
                    if (StdOutDebug) std::cout  << "Received " << bytes << " bytes from UART" << std::endl;
                    if (StdOutDebug) std::cout  << Utils::ConvertToHex(m_buffer, bytes) << std::endl;
                    
                    // Parse serial data and construct new packet
                    for(int i = 0; i < (int)bytes; i++)
                    {
                        if(m_buffer[i] == (uint8_t)START_OF_PACKET)
                        {
                            // Reset for new packet
                            memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                            m_packetLen = 0;
                        }

                        // Add new byte into tail of packet
                        if(m_packetLen < SERIAL_PACKET_LENGTH)
                            m_packet[m_packetLen++] = m_buffer[i];
                            
                        // Check if we get whole packet
                        if(m_packetLen >= SERIAL_PACKET_LENGTH)
                        {
                            if(m_packet[0] == (uint8_t)START_OF_PACKET && m_packet[SERIAL_PACKET_LENGTH - 1] == (uint8_t)END_OF_PACKET)
                            {
	                            //Log Incoming Serial Data
	                            char character_data[SERIAL_PACKET_LENGTH];
	                            for (int ii = 0;ii < SERIAL_PACKET_LENGTH;ii++)
	                            {
		                            character_data[ii] = (char) m_packet[ii];
	                            }
	                            std::string message(character_data, (size_t) m_packetLen);
			
	                            std::string hexMessage = Utils::string_to_hex(message);
					
	                            spdlog::get("E23DataLog")->info() << "IN :" << hexMessage;
                                //spdlog::get("E23DataLog")->info("IN: {}", hexMessage);
	                            
	                            //Check if packet should be processed and handled locally and also if is a valid packet to be sent to end point.
	                            if (!(ProcessLocalPackets(m_packet, m_packetLen)))
	                            {
		                            // We got a valid packet to be sent to end point.  
		                            m_DataReceived(m_packet, m_packetLen);
	                            }                                                                                            
                            }
                            else
                            {
                                // Invalid packet
                                if (StdOutDebug) std::cout  << "Received invalid packet from external device" << std::endl;
                                //if (StdOutDebug) std::cout  << Utils::ConvertToHex(m_packet, m_packetLen) << std::endl;
                            }
                            // Reset for new packet
                            memset(m_packet, 0L, SERIAL_PACKET_LENGTH + 1);
                            m_packetLen = 0;
                        }
                    }
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() << "' when reading data from external device" << std::endl;
                }
                
                if(err != asio::error::operation_aborted)
                {
                    // Start next asynchonous read operation
                    StartAsyncRead();
                }
            }
            
            void HandleWrite(const asio::error_code& err, std::size_t bytes)
            {
                if(!err)
                {
                    if (StdOutDebug) std::cout  << bytes <<  " bytes were sent successfully" << std::endl;
                    
                    // Remote processed packet from queue
                    m_writePackets.pop_front();
                    if(!m_writePackets.empty())
                    {
                        if (StdOutDebug) std::cout  << "Sending bytes: " << Utils::ConvertToHex(m_writePackets.front().Data(), m_writePackets.front().Length()) << " to UART port" << std::endl;
                        
                        // Send queued packet to serial port now
                        asio::async_write(m_serialPort, asio::buffer(m_writePackets.front().Data(), m_writePackets.front().Length()), 
                            boost::bind(&Serial::HandleWrite, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                    }
                }
                else
                {
                    std::cerr << "Failed: '" << err.message() << "' when writing data to external device" << std::endl;
                }
            }
            
            void HandleSignal(const std::error_code err, int signalNnumber)
            {
                if(!err)
                {
                    //std::cout << "Received signal: " << std::to_string(signalNnumber) << std::endl;
                    if(signalNnumber == SIGINT)
                        spdlog::get("E23DataLog")->info() << "Received SIGINT signal. Prepare to stop the application";
                    else if(signalNnumber == SIGTERM)
                        spdlog::get("E23DataLog")->info() << "Received SIGTERM signal. Prepare to stop the application";
                    else
                    {
                        spdlog::get("E23DataLog")->info() << "Received signal: " << std::to_string(signalNnumber) << ". Ignore it now";
                        return;
                    }
                    
                    // Close COM port
                    CloseSerial();
                    // Post message into Asio service to stop it
                    m_serialPort.get_io_service().stop();
                }
            }
	        
            void InitializeGPIOPin()
            {
                // Set PIN as digital input
                digital_direction(HEADER, PIN, IN);
                
                // Reset variables
                m_levelShifter = 0x00;
                m_levelCounter = 0;
    
                // Start timer to read GPIO pin periodically
                m_GPIOReadTimer.expires_from_now(boost::posix_time::millisec(GPIO_PIN_READ_PERIOD));
                m_GPIOReadTimer.async_wait(boost::bind(&Serial::ReadGPIOPin, this));
            }
            
            void ReadGPIOPin()
            {
                unsigned char currentlevel;
                
                // Read the level of the pin.
                currentlevel = digital_input(HEADER, PIN);
                
                if(currentlevel == 1)
                {
                    m_levelShifter = (m_levelShifter << 1) + 1;
                }
                else
                {
                    m_levelShifter = (m_levelShifter << 1);
                }
                m_levelCounter++;
                
                if(m_levelCounter == DEBOUNCE_SAMPLES)
                {
                    if(m_levelShifter == 0x00)  // We have got DEBOUNCE_SAMPLES LOW samples continuously
                    {
                        if(m_prevState == 1)  // We need to reset the application
                        {
                            spdlog::get("E23DataLog")->info() << "Received transition from HIGH to LOW on desire GPIO pin. Prepare to stop the application";
                            // Delay 3s before to stop the application
                            m_GPIOReadTimer.expires_from_now(boost::posix_time::seconds(3));
                            m_GPIOReadTimer.wait();
                            // Close COM port
                            CloseSerial();
                            // Post message into Asio service to stop it
                            m_serialPort.get_io_service().stop();
                            return;
                        }
                    }
                    else if(m_levelShifter == HIGH_SAMPLES_MASK) // We have got DEBOUNCE_SAMPLES HIGH samples continuously
                    {
                        if(m_prevState == 0)
                        {
                            m_prevState = 1;
                        }
                    }
                    else    // We have noises and we don't change the state
                    {
                    	if (StdOutDebug) std::cout << "Get noises from GPIO pin: " << std::endl;
                    }
                    
                    // Reset variables
                    m_levelShifter = 0x00;
                    m_levelCounter = 0;
                }

                // Re-start timer to read GPIO pin periodically
                m_GPIOReadTimer.expires_from_now(boost::posix_time::millisec(GPIO_PIN_READ_PERIOD));
                m_GPIOReadTimer.async_wait(boost::bind(&Serial::ReadGPIOPin, this));
            }
            
	        bool ProcessLocalPackets(uint8_t* pPacket, uint8_t len)
	        {
		        
		        int sendreply = false;
		        char IPAddress[8 + 1];
		        char *IPAddrPtr;
		        char DateTimeStr[12 + 1];
		        char *DateTimePtr;
		        
		        memcpy(m_loopbuffer, pPacket, len);
		        m_loopbufferLen = len;
		        	                       	
		        if (m_loopbufferLen == SERIAL_PACKET_LENGTH)
		        {
			        if (m_loopbuffer[1] == 0xE1)
			        {
				        return true;				//Ignore these commands and do not reply to them
			        }
			        
			        if (m_loopbuffer[1] == 0xE2)
			        {
						return true;			   //Ignore these commands and do not reply to them
					}
			     
			        if (m_loopbuffer[1] == 0xE6)  // Cycle Thru Commands Below
			        {
				        lcd_update_cntr++;
				        switch(lcd_update_cntr)
				        {
							case 1:
									m_loopbuffer[1] = 0xEA;
									break;
							case 2:
									m_loopbuffer[1] = 0xEB;
									break;
							case 3:
									m_loopbuffer[1] = 0xEC;
									break;
							case 4:
									m_loopbuffer[1] = 0xED;
									break;					       
							case 5:
									//Attempt Non Requested Time Synch to QRFL only twice after starting program, Requested Time Synchs that occur when the QRFL is reset can be done as many times are required
									if (time_set > 0)
									{
										m_loopbuffer[1] = 0xEE;	
										time_set--;
									}
									else
									{
										m_loopbuffer[1] = 0xE5;									
									}
									break;
							case 6:
									//Attempt Non Requested Date Synch to QRFL only twice after starting program, Requested Date Synchs that occur when the QRFL is reset can be done as many times are required									
									if (date_set > 0)
									{
										m_loopbuffer[1] = 0xEF;	
										date_set--;
									}
									else
									{
										m_loopbuffer[1] = 0xE5;									
									}
									break;							
							default:
									if ((tmp_cntr % 5) == 0)
									{
										m_loopbuffer[2] = 0x30;   // 0x30, 0x30 indictes offline
										m_loopbuffer[3] = 0x30;				            
									}
									else
									{
										m_loopbuffer[2] = 0x31;   // 0x31, 0x31 indictes online
										m_loopbuffer[3] = 0x31;
									}
									m_loopbuffer[4] = 0x0D;
									m_loopbuffer[5] = 0x0A;
									sendreply = true;
									lcd_update_cntr = 0;
				        
				        }				        					    				        
			        }
			        
			        if (m_loopbuffer[1] == 0xE5) 
			        {
				        tmp_cntr++;				//Temporary Toggle of ON line and Offline
				        if ((tmp_cntr % 5) == 0)
				        {
					        m_loopbuffer[2] = 0x30;   // 0x30, 0x30 indictes offline
					        m_loopbuffer[3] = 0x30;				            
				        }
				        else
				        {
					        m_loopbuffer[2] = 0x31;   // 0x31, 0x31 indictes online
					        m_loopbuffer[3] = 0x31;
				        }
				        m_loopbuffer[4] = 0x0D;
				        m_loopbuffer[5] = 0x0A;
				        sendreply = true;
			        } 
			        
			        if (m_loopbuffer[1] == 0xEA) // Software Version V1R23
			        {
				        m_loopbuffer[2] = 0x56;
				        m_loopbuffer[3] = 0x31;
				        m_loopbuffer[4] = 0x52;
				        m_loopbuffer[5] = 0x32;
				        m_loopbuffer[6] = 0x33;
				        m_loopbuffer[7] = 0x0D;
				        m_loopbuffer[8] = 0x0A;
				        sendreply = true;
			
			        }
			        
			        if (m_loopbuffer[1] == 0xEB)   //Latency  0x0F (15 x 10ms) = 150ms
			        {
				        m_loopbuffer[2] = 0x30;
				        m_loopbuffer[3] = 0x3F;
				        m_loopbuffer[4] = 0x30;		//Need to repeat it twice
				        m_loopbuffer[5] = 0x3F;
				        m_loopbuffer[6] = 0x30;
				        m_loopbuffer[7] = 0x0D;
				        m_loopbuffer[8] = 0x0A;
				        sendreply = true;
			        }
			        
			        if ((m_loopbuffer[1] == 0xEC)  || (m_loopbuffer[1] == 0xED))   // IP Commands
			        {
				        IPAddrPtr = SafeEngineering::Utils::GetIPAddressLCDString(IPAddress, sizeof(IPAddress));	    	    
				        if (StdOutDebug) std::cout  << "IP ADDRESS CALCULATION : " << IPAddress << std::endl;
			       			        
				        if (m_loopbuffer[1] == 0xEC)  // IP Part A and B  192.168
				        {
					        m_loopbuffer[2] = IPAddress[0];
					        m_loopbuffer[3] = IPAddress[1];
					        m_loopbuffer[4] = IPAddress[2];
					        m_loopbuffer[5] = IPAddress[3];
					        m_loopbuffer[6] = 0x30;
					        m_loopbuffer[7] = 0x0D;
					        m_loopbuffer[8] = 0x0A;
					        sendreply = true;
				        }
			        
				        if (m_loopbuffer[1] == 0xED) // IP Part C and D  236.201
				        {
					        m_loopbuffer[2] = IPAddress[4];
					        m_loopbuffer[3] = IPAddress[5];
					        m_loopbuffer[4] = IPAddress[6];
					        m_loopbuffer[5] = IPAddress[7];
					        m_loopbuffer[6] = 0x30;
					        m_loopbuffer[7] = 0x0D;
					        m_loopbuffer[8] = 0x0A;
					        sendreply = true;
				        }
			        }
			        
			        
			        if ((m_loopbuffer[1] == 0xEE)  || (m_loopbuffer[1] == 0xEF))   // Date/Time Commands
			        {
				        DateTimePtr = SafeEngineering::Utils::GetDateTimeLCDString(DateTimeStr, sizeof(DateTimeStr), std::chrono::system_clock::now());	    	    
				        if (StdOutDebug) std::cout  << "DATE TIME CALCULATION : " << DateTimeStr << std::endl;
				        
				        if (m_loopbuffer[1] == 0xEE) //Time HH MM SS,
				        {
				        	m_loopbuffer[2] = DateTimeStr[0];
					        m_loopbuffer[3] = DateTimeStr[1];
					        m_loopbuffer[4] = DateTimeStr[2];
					        m_loopbuffer[5] = DateTimeStr[3];
					        m_loopbuffer[6] = DateTimeStr[4];
					        m_loopbuffer[7] = DateTimeStr[5];
				        	m_loopbuffer[8] = 0x0A;
					        sendreply = true;
				        }
			        
				        if (m_loopbuffer[1] == 0xEF) //Date YY - MM - DD
				        {
					        m_loopbuffer[2] = DateTimeStr[6];
					        m_loopbuffer[3] = DateTimeStr[7];
					        m_loopbuffer[4] = DateTimeStr[8];
					        m_loopbuffer[5] = DateTimeStr[9];
					        m_loopbuffer[6] = DateTimeStr[10];
					        m_loopbuffer[7] = DateTimeStr[11];
					        m_loopbuffer[8] = 0x0A;
					        sendreply = true;
				        }
			        }		            
#if SIM_MASTER_MODE
			        
			        if (m_loopbuffer[1] == 0xCB)
			        {
				        m_loopbuffer[1] = 0x6F;
				        if ((tmp_cntr % 25) == 0)
				        {
					        m_loopbuffer[2] = 0x01;
					        sendreply = true;
				        }
				        else
				        {
					        m_loopbuffer[2] = 0x7D;
				        }	
				        m_loopbuffer[3] = 0x00;
				        sendreply = true;
			        }
		            
		            
			        if (m_loopbuffer[1] == 0xF1)
			        {
				        tmp_cntr++;
				        if (tmp_cntr == 15)
				        {
					        m_loopbuffer[1] = 0x6E;    //Send TRIP ALARM
				        }
				        else
				        {
					        m_loopbuffer[1] = 0x6F;
				        }
						
				        m_loopbuffer[2] = 0x51;
				        m_loopbuffer[3] = 0x36;
				        m_loopbuffer[4] = 0x30;
				        m_loopbuffer[5] = 0x30;
				        m_loopbuffer[6] = 0x3A;
				        m_loopbuffer[7] = 0x3D;
				        sendreply = true;
			        }
		            
		            
			        if (m_loopbuffer[1] == 0xF9)
			        {
				        m_loopbuffer[1] = 0x6F;
				        m_loopbuffer[2] = 0x59;
				        m_loopbuffer[3] = 0x31;
				        m_loopbuffer[4] = 0x30;
				        m_loopbuffer[5] = 0x30;
				        m_loopbuffer[6] = 0x30;
				        m_loopbuffer[7] = 0x38;
				        sendreply = true;
			        }
		            
		            
			        if (m_loopbuffer[1] == 0xC1)
			        {
				        m_loopbuffer[1] = 0x6F;
				        m_loopbuffer[2] = 0x41;
				        m_loopbuffer[3] = 0x31;
				        m_loopbuffer[4] = 0x34;
				        if ((fault_cntr % 3) == 0)
				        {
					        m_loopbuffer[5] = 0x38;   //Correct    
				        }
				        else
				        {
					        m_loopbuffer[5] = 0x30;  //Error	        
				        }
				        fault_cntr++;
				        
				        m_loopbuffer[6] = 0x3C;
				        m_loopbuffer[7] = 0x00;
				        sendreply = true;
			        }
		            
		            
			        if (m_loopbuffer[1] == 0xC9)
			        {
				        m_loopbuffer[1] = 0x6F;
				        m_loopbuffer[2] = 0x49;
				        m_loopbuffer[3] = 0x31;
				        m_loopbuffer[4] = 0x34;
				        m_loopbuffer[5] = 0x3E;
				        m_loopbuffer[6] = 0x32;
				        m_loopbuffer[7] = 0x00;
				        sendreply = true;
			        }
			       
#endif			        
					
		        }
	            	         
		        if (sendreply)
		        {		            
					// Send data to UART port
			        if (StdOutDebug) std::cout  << "Forward packet to external UART device" << std::endl;
			        SendPacket(m_loopbuffer, m_loopbufferLen);            
		        }  
		        		        
		        return sendreply;			        
			        
	        }
            
        private:
            // Object describes COM port from ASIO library
            asio::serial_port m_serialPort;
            
            // Buffer holds data received from COM port
            uint8_t m_buffer[SERIAL_BUFFER_LENGTH + 1];
            
            // Data packet from external device
            uint8_t m_packet[SERIAL_PACKET_LENGTH + 1];
            uint8_t m_packetLen;
            
            // The queue holds packets needed writting to hardware serial port
            std::deque<SerialPacket> m_writePackets;
	        
	        // Locally processed Data packets
	        uint8_t m_loopbuffer[SERIAL_BUFFER_LENGTH + 1];
	        uint8_t m_loopbufferLen;
	        
	        //State Variable to Periodically Update the QRFL LCD Display
	        int lcd_update_cntr = 0;
	        
	        int date_set = 2;	//Set Date Twice on Startup
	        int time_set = 2;	//Set Time Twice on Startup
	        
	        //Temporary Test Counter variable
	        int tmp_cntr = 1;
	        int fault_cntr = 1;
	        
	        bool StdOutDebug = false;
	        	   
            // Catch terminated events
            asio::signal_set m_signalSet;     
            
            // Timer used to read pin GPIO
            asio::deadline_timer m_GPIOReadTimer;
            // Initial state of pin
            unsigned char m_prevState = 0;
            
            // Shift-register holds continuelly levels 
            unsigned char m_levelShifter;
            unsigned char m_levelCounter;
                            
        };  // Serial class
        
    }   // namespace Comm
} 

#endif // SERIAL_HPP
