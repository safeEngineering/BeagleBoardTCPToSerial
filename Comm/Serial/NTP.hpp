#ifndef NTP_HPP
#define NTP_HPP

#include <iostream>
#include <iomanip>
#include <deque>
#include <string>
#include <cmath>

#include "spdlog/spdlog.h"
#include "DailyFileSink.hpp"

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <asio.hpp>

#define NTP_POLL_INTERVAL           15
#define NTP_POLL_ACCEPTABLE_ONLINE_COUNT   2
#define NTP_POLL_MAX_ONLINE_COUNT 5

namespace SafeEngineering
{
    namespace Comm
    {
        class NTP
        {
        public:
	        NTP(asio::io_service& io, bool consoleDebug, const std::string& ntpIP)
		        : m_NtpTimer(io)
		        , StdOutDebug(consoleDebug)
		        , m_NtpResponseTimer(io)
		        , m_NtpSocket(io, asio::ip::udp::v4())
		        , m_NtpIP(ntpIP)
	        {
		        		        
		        asio::ip::udp::resolver resolNtp(io);
		        asio::ip::udp::resolver::query queryNtp(asio::ip::udp::v4(), ntpIP, "123");
		        m_NtpEP = *resolNtp.resolve(queryNtp);
	            
		        m_NtpActive = 0;
		        		        
	        }
	        
	        ~NTP()
	        {
	        }
            	        	        
			// Start periodially timer used to NTP
	        void InitializeNTP()
	        {	            
				// Initialize periodically NTP check
		        m_NtpTimer.expires_from_now(boost::posix_time::seconds(NTP_POLL_INTERVAL));
		        m_NtpTimer.async_wait(boost::bind(&NTP::HandleNTPInterval, this));
	        }
	        
	        void StopNTP()
	        {	            
		        m_NtpTimer.cancel();
		        m_NtpSocket.cancel();		        
	        }
            	                    	        
			// Process periodically timer and start NTP Check
	        void HandleNTPInterval()
	        {    
		        // Open the socket - this is required in case the network has become unavailable (i.e plug diconnected) in the mean time.		        
		        asio::error_code ec;
		        			        
		        if (m_NtpSocket.is_open())
		        {
			        m_NtpSocket.close(ec);
		        }
		        
		        m_NtpSocket.open(asio::ip::udp::v4(), ec);
		        if (ec)
		        {
			        std::string msg = "NTP Socket Open Error ";
			        std::cout << msg << std::endl;			            
		        }
		        else
		        {	
			        boost::array<unsigned char, 48> sendBuf = {010, 0, 0, 0, 0, 0, 0, 0, 0};
			        
			        std::string msg = "Sending Request to NTP :" + m_NtpIP;
			        if (StdOutDebug) std::cout << msg << std::endl;
			        spdlog::get("E23DataLog")->info() << msg;
			        			        
			        m_NtpSocket.send_to(asio::buffer(sendBuf), m_NtpEP);

			        asio::ip::udp::endpoint sender_endpoint;
			        
			        // Wait up to five seconds for a reply.
			        m_NtpTimer.expires_from_now(boost::posix_time::seconds(2));
			        m_NtpTimer.async_wait(boost::bind(&NTP::HandleNtpTimeout, this));
                
			        m_NtpRxValidPacket = false;
					// Start receiving.
					// First, discard any data already in the buffer.
			        m_NtpReplyBuffer.fill(0u);
			        // Next, wait for a reply
			        m_NtpSocket.async_receive(asio::buffer(m_NtpReplyBuffer), boost::bind(&NTP::HandleNtpResponse, this, _2)) ;							        
		        }
				        			     	                        
	        }
	        	        
	        // Process ping reply from gateway
	        void HandleNtpResponse(std::size_t length)
	        {
	            		        
		        m_NtpTimer.cancel();
		        		        		        
		        if (length == 48)  //Valid NTp Packet size
		        {			        		        
			        time_t secs = ntohl(m_NtpReplyBuffer[8]) - 2208988800u;
			        tm *z = localtime(&secs);
			        double secfrac = (double)ntohl(m_NtpReplyBuffer[9]) / 4294967296.0;
			        			        
			        if (m_NtpActive < NTP_POLL_MAX_ONLINE_COUNT)
			        {
				        m_NtpActive++;    
			        }
			        
			        m_NtpRxValidPacket = true;
			        
			        std::string msg = "Received reply from NTP " + m_NtpIP;
			        if (StdOutDebug) std::cout << msg << std::endl;
			        spdlog::get("E23DataLog")->info() << msg;
			        
			        if (StdOutDebug) std:: cout << z->tm_mday << '.' << z->tm_mon + 1 << '.'
			                   << z->tm_year + 1900 << "   "
			                   << z->tm_hour << ':' << z->tm_min << ':'
			                   << secfrac + z->tm_sec << std::endl;
			        
		        }			        		        
	        }
            
	        // Handle timeout of Ntp command
	        void HandleNtpTimeout()
	        {
		        if (!m_NtpRxValidPacket)
		        {
			        if (m_NtpActive > 0)
			        {
				        m_NtpActive--;    
			        }			        
		        }
		        
				//Close the Ping Socket
		        m_NtpSocket.cancel();
		        if (m_NtpSocket.is_open())
		        {
			        asio::error_code ec;
			        m_NtpSocket.close(ec);
		        }
		        
		        InitializeNTP();  //Restart the Whole Sequence Again 		        
	        }
            
	        // Numbers of successful NTP commands
	        unsigned char m_NtpActive;
	        
	        private:
			 
				 // NTP IP address
				std::string m_NtpIP;
				// Timer used to initiate NTP
				asio::deadline_timer m_NtpTimer;
				// Timer used to wait for NTP response
				asio::deadline_timer m_NtpResponseTimer;            
				// Socket used to send NTP packets
				asio::ip::udp::socket m_NtpSocket;
				// Endpoint of NTP
				asio::ip::udp::endpoint m_NtpEP;
				// Buffer holds reply message from NTP
				boost::array<unsigned long, 1024> m_NtpReplyBuffer;
			
				bool m_NtpRxValidPacket;
	        
				bool StdOutDebug = false;
	        	                                            
			}
			;  // NTP class
        
		}   // namespace Comm
	} 

#endif //NTP_HPP
		        
		        
        