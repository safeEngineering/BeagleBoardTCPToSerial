#ifndef PING_HPP
#define PING_HPP

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

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

#define GATEWAY_PING_INTERVAL           15
#define GATEWAY_PING_NUMBERS            5
#define GATEWAY_PING_ACCEPTABLE_ONLINE_COUNT   3

namespace SafeEngineering
{
    namespace Comm
    {
        class PING
        {
        public:
	        PING(asio::io_service& io, bool consoleDebug, const std::string& gatewayIP)
		        : m_ioService(io)
		        , m_PingTimer(io)
		        , StdOutDebug(consoleDebug)
				, m_GateWayIP(gatewayIP)
		        , m_ResponseTimer(io)
		        , m_PingSocket(io, asio::ip::icmp::v4())
	        {		        		        
		        // Resolve IP address of gateway
                asio::ip::icmp::resolver resol(io);
                asio::ip::icmp::resolver::query query(asio::ip::icmp::v4(), gatewayIP, "");
                m_GatewayEP = *resol.resolve(query);
	            m_PingActive = 0;		        		        
	        }
	        
	        ~PING()
	        {
	        }
	        
	        void SetIPAddress(std::string GatewayIPAddress)	
	        {
		        asio::ip::icmp::resolver resol(m_ioService);
		        asio::ip::icmp::resolver::query query(asio::ip::icmp::v4(), GatewayIPAddress, "");
		        m_GatewayEP = *resol.resolve(query);	
		        m_GateWayIP = GatewayIPAddress;
	        }
            	        	        
			// Start periodially timer used to ping gateway
            void InitializePing()
            {	            
	            // Initialize periodically ping timer
                m_PingTimer.expires_from_now(boost::posix_time::seconds(GATEWAY_PING_INTERVAL));
                m_PingTimer.async_wait(boost::bind(&PING::HandlePingInterval, this));
            }
	        
	        void StopPing()
	        {	            
		        is_running = false;
		        
		        try
		        {		      			        			       
			        m_PingSocket.cancel();
		        }
				catch (const std::exception& ex)
				{
					std::cerr << "STOP Socket Ping Error " <<  ex.what() << std::endl;
		        }
		        
		        try
		        {		      			       
			        m_PingTimer.cancel();
		        }
		        catch (const std::exception& ex)
		        {
			        std::cerr << "STOP Timer Ping Error " <<  ex.what() << std::endl;
		        }
		        		        
		        try
		        {		      			       
			        m_ResponseTimer.cancel();
		        }
		        catch (const std::exception& ex)
		        {
			        std::cerr << "STOP Timer Ping Error " <<  ex.what() << std::endl;
		        }
		        		        
		        
	        }
            	                    	        
            
            // Process periodically timer and start first ping command
            void HandlePingInterval()
            {                
                // Reset numbers
                m_SequenceNumber = 0;
	            
	            if (!is_running) return;
                
                // Send first ping command of session
                SendPingCommand();
            }
            
	        // Send asynchronously Ping packet to gateway
	        void SendPingCommand()
	        {
		        std::string body("\"Hello!\" from SerialTCP ping.");
                
		        std::string msg = "Send Ping command to gateway " + m_GateWayIP;
		        if (StdOutDebug) std::cout << msg << std::endl;
		        //spdlog::get("dump_data_log")->info() << msg;
                
		        // Create an ICMP header for an echo request.
		        icmp_header echo_request;
		        echo_request.type(icmp_header::echo_request);
		        echo_request.code(0);
		        echo_request.identifier(static_cast<unsigned short>(::getpid()));
		        echo_request.sequence_number(++m_SequenceNumber);
		        compute_checksum(echo_request, body.begin(), body.end());
    
		        // Encode the request packet.
		        asio::streambuf request_buffer;
		        std::ostream os(&request_buffer);
		        os << echo_request << body;
    
				// Open the socket - this is required in case the network has become unavailable (i.e plug diconnected) in the mean time.		        
		        asio::error_code ec;
		        if (m_PingSocket.is_open())
		        {
			        m_PingSocket.close(ec);
		        }
		        m_PingSocket.open(asio::ip::icmp::v4(), ec);
		        if (ec)
		        {
			        std::string msg = "Ping Socket Open Error ";
			        if (StdOutDebug) std::cout << msg << std::endl;
			        spdlog::get("dump_data_log")->info() << msg;    
		        }
		        else
		        {			        		        
			        // Send the request.
			        m_PingSocket.send_to(request_buffer.data(), m_GatewayEP);

					// Wait up to five seconds for a reply.
					m_ResponseTimer.expires_from_now(boost::posix_time::seconds(2));
					m_ResponseTimer.async_wait(boost::bind(&PING::HandlePingTimeout, this));
			        
			        m_PingRxValidPacket = false;                
					// Start receiving.
					// First, discard any data already in the buffer.
					m_ReplyBuffer.consume(m_ReplyBuffer.size());
					// Next, wait for a reply. We prepare the buffer to receive up to 64KB.
					m_PingSocket.async_receive(m_ReplyBuffer.prepare(65536), boost::bind(&PING::HandlePingResponse, this, _2));
				}
            }
            
            // Process ping reply from gateway
            void HandlePingResponse(std::size_t length)
            {
                // The actual number of bytes received is committed to the buffer so that we
                // can extract it using a std::istream object.
                m_ReplyBuffer.commit(length);
            
                // Decode the reply packet.
                std::istream is(&m_ReplyBuffer);
                ipv4_header ipv4_hdr;
                icmp_header icmp_hdr;
                is >> ipv4_hdr >> icmp_hdr;
                
                // We can receive all ICMP packets received by the host, so we need to
                // filter out only the echo replies that match the our identifier and
                // expected sequence number.
                if (is && icmp_hdr.type() == icmp_header::echo_reply
                       && icmp_hdr.identifier() == static_cast<unsigned short>(::getpid())
                       && icmp_hdr.sequence_number() == m_SequenceNumber)
                {
                    // Interrupt response timer
                    m_ResponseTimer.cancel();
                    std::string msg = "Received reply from gateway " + m_GateWayIP;
                    if (StdOutDebug) std::cout << msg << std::endl;
	                //spdlog::get("dump_data_log")->info() << msg;
	                if (m_PingActive < GATEWAY_PING_NUMBERS)
	                {
		                m_PingActive++;
	                }
	                m_PingRxValidPacket = true;
                }
            }
            
            // Handle timeout of Ping command
            void HandlePingTimeout()
            {
	            if (!m_PingRxValidPacket)
	            {
		            if (m_PingActive > 0)
		            {
			            m_PingActive--;    
		            }			        
	            }
	            
				//Close the Ping Socket
	            m_PingSocket.cancel();
	            if (m_PingSocket.is_open())
	            {
		            asio::error_code ec;
		            m_PingSocket.close(ec);
	            }
	            
	            if (!is_running) return;
	            
                if(m_SequenceNumber < GATEWAY_PING_NUMBERS)
                {
                    // Requests must be sent no less than one second apart.
                    m_ResponseTimer.expires_from_now(boost::posix_time::seconds(1));
                    m_ResponseTimer.async_wait(boost::bind(&PING::SendPingCommand, this));
                }
                else
                {                    
	                std::string msg = "Received " + std::to_string(m_PingActive) + " replies among of " + std::to_string(GATEWAY_PING_NUMBERS) + " Ping commands";
	                if (StdOutDebug) std::cout << msg << std::endl;
	                //spdlog::get("dump_data_log")->info() << msg;
	                
	                InitializePing();  //Restart the Whole Sequence Again 
                }
            }
            
	         // Numbers of successful ping commands
	        unsigned char m_PingActive;
	        
	        private:
				asio::io_service& m_ioService;
				
				// Gateway IP address
				std::string m_GateWayIP;
				// Timer used to ping gateway
				asio::deadline_timer m_PingTimer;
				// Timer used to wait for response
				asio::deadline_timer m_ResponseTimer;
				// Socket used to send ping packets
				asio::ip::icmp::socket m_PingSocket;
				// Endpoint of gateway
				asio::ip::icmp::endpoint m_GatewayEP;
				// Buffer holds reply message from gateway
				asio::streambuf m_ReplyBuffer;
	        
				bool m_PingRxValidPacket;
            						
				bool StdOutDebug = false;
	        
				bool is_running = true;
			
				// Numbers of total ping commands
				unsigned char m_SequenceNumber;
           														
			}
			;  // PING class
        
		}   // namespace Comm
	} 

#endif //PING_HPP
		        
		        
        