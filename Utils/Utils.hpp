#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <iostream>
#include <fstream>
#include <array>
#include <chrono>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include <json.hpp>

#define MAX_REMOTE_UNIT_NUMBERS            10

namespace SafeEngineering
{
    namespace Utils
    {
        typedef enum
        {
            UNUSED      = 0x00,
            MASTER      = 0x01,
            SUBMASTER   = 0x02,
            SLAVE       = 0x04
        } UnitType;
        
        typedef struct
        {
            // ID of unit
            int ID;
            // Type of unit (master, submaster, slave)
            UnitType Type;
            // IP address
            std::string IPAddress;
        } Unit;
        
        // The structure holds settings from settings.json file
        typedef struct
        {
            // Not used yet
            std::string Version;
            // Parameters of current unit
	        std::string SiteName;
	        Unit CurrentUnit;
            // Gateway IP address
            std::string GatewayIPAddr;
            // NTP Server IP address
	        std::string NTPServerIPAddr;
	        // Parameters of the remote boards in the system
            std::array<Unit, MAX_REMOTE_UNIT_NUMBERS> Units;
        } Settings;
	    
	    inline char* GetIPAddressLCDString(char* IPAddress, size_t sizeIPAddress)
	    {
	    
		    struct ifaddrs *ifap, *ifa;
		    struct sockaddr_in *sa;
            char *addr;
		    
		    //Initialise to All Zeros
		    if (sizeIPAddress > 0)
		    {			    		    
				for (int i = 0; i < ((int)sizeIPAddress - 1); i++)
				{
					IPAddress[i] = 0x30;
				}
				IPAddress[sizeIPAddress - 1] = 0x00;
			}	    
		    
		    try
		    {		    
			    getifaddrs(&ifap);
			    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
				    if (ifa->ifa_addr->sa_family == AF_INET) {
					    sa = (struct sockaddr_in *) ifa->ifa_addr;
					    addr = inet_ntoa(sa->sin_addr);
					    if (strncmp(ifa->ifa_name, "eth0", 4) == 0)
					    {
						    unsigned long ipA = (unsigned long) sa->sin_addr.s_addr;
						    unsigned char ipaddr[4];
					    
						    ipaddr[0] = (unsigned char) ipA % 256;
						    ipA >>= 8;
						    ipaddr[1] = (unsigned char) ipA % 256;
						    ipA >>= 8;
						    ipaddr[2] = (unsigned char) ipA % 256;
						    ipA >>= 8;
						    ipaddr[3] = (unsigned char) ipA % 256;
						    //printf("Interface: %s\tAddress: %s : \n", ifa->ifa_name, addr);    
						    //printf("IP %ul {%03d.%03d.%03d.%03d} \n", ipA, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);    
						
						    if (sizeIPAddress > 8)
						    {						    					    
							    IPAddress[0] = ((ipaddr[0] & 0xF0) >> 4) + 0x30;
							    IPAddress[1] = (ipaddr[0] & 0x0F) + 0x30;
							    IPAddress[2] = ((ipaddr[1] & 0xF0) >> 4) + 0x30;
							    IPAddress[3] = (ipaddr[1] & 0x0F) + 0x30;
							    IPAddress[4] = ((ipaddr[2] & 0xF0) >> 4) + 0x30;
							    IPAddress[5] = (ipaddr[2] & 0x0F) + 0x30;
							    IPAddress[6] = ((ipaddr[3] & 0xF0) >> 4) + 0x30;
							    IPAddress[7] = (ipaddr[3] & 0x0F) + 0x30;
							    IPAddress[8] = 0x00;
					    
								//printf("LCD IP %s \n", IPAddress);    					    
						    }
					    }				    
				    }
			    }

			    freeifaddrs(ifap);
		    }
		    catch (const std::exception& ex)
		    {
			    std::cerr << "GetIPAddressLCDString stopped with an exception: " << ex.what();		    
		    }
		    catch (...)
		    {
			    std::cerr << "GetIPAddressLCDString stopped with unexpected exception";		    
		    }
		    
		    return IPAddress;
	    }
	    
	    inline char* GetDateTimeLCDString(char* DateTimeStr, size_t sizeDateTime, const std::chrono::system_clock::time_point& tp)
	    {
		    //Initialise to All Zeros
		    if (sizeDateTime > 0)
		    {			    		    
			    for (int i = 0; i < ((int)sizeDateTime - 1); i++)
			    {
				    DateTimeStr[i] = 0x30;
			    }
			    DateTimeStr[sizeDateTime - 1] = 0x00;
		    }	    
		    
		    time_t t = std::chrono::system_clock::to_time_t(tp);
		    tm* ptm = std::localtime(&t);

		    if (sizeDateTime > 12)
		    {						    					    
			    DateTimeStr[0] = (ptm->tm_hour / 10) + 0x30;
			    DateTimeStr[1] = (ptm->tm_hour % 10) + 0x30;
			    DateTimeStr[2] = (ptm->tm_min  / 10) + 0x30;
			    DateTimeStr[3] = (ptm->tm_min % 10) + 0x30;
			    DateTimeStr[4] = (ptm->tm_sec / 10) + 0x30;
			    DateTimeStr[5] = (ptm->tm_sec % 10) + 0x30;
			    DateTimeStr[6] = ((ptm->tm_year - 100) / 10) + 0x30;  //need to subtract 100 as tm_years is num years since 1900
			    DateTimeStr[7] = ((ptm->tm_year - 100) % 10) + 0x30;  //need to subtract 100 as tm_years is num years since 1900	
			    DateTimeStr[8] = ((ptm->tm_mon + 1) / 10) + 0x30;			//need to add 1 as tm_mon is 0 thru 11 and we need 1 thru 12
			    DateTimeStr[9] = ((ptm->tm_mon + 1) % 10) + 0x30;
			    DateTimeStr[10] = (ptm->tm_mday / 10) + 0x30;
			    DateTimeStr[11] = (ptm->tm_mday % 10) + 0x30;			    
			    DateTimeStr[12] = 0x00;
				
			    //printf("LCD DateTime %d %d %d   %d %d %d \n", ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ptm->tm_year, ptm->tm_mon, ptm->tm_mday);
				//printf("LCD DateTime %s \n", DateTimeStr);    					    
		    }
		    		    
		    return DateTimeStr;
	    }
	    
	    inline std::string timeString(const std::chrono::system_clock::time_point& tp)
	    {
		    time_t t = std::chrono::system_clock::to_time_t(tp);
		    tm* ptm = std::localtime(&t);
		    char szDateBuffer[128];
		    size_t nStartLength = strftime(szDateBuffer, 128, "[%d-%m-%Y %H:%M:%S.", ptm);

		    auto duration = tp.time_since_epoch();
		    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

		    size_t nEndLength = sprintf(&szDateBuffer[nStartLength], "%03u]", (unsigned int)millis);

		    return std::string(szDateBuffer, nStartLength + nEndLength);
	    }

	    inline std::string string_to_hex(const std::string& input)
	    {
		    static const char* const lut = "0123456789ABCDEF";
		    size_t len = input.length();

		    std::string output;
		    output.reserve(2 * len);
		    for (size_t i = 0; i < len; ++i)
		    {
			    const unsigned char c = input[i];
			    output.push_back('<');
			    output.push_back(lut[c >> 4]);
			    output.push_back(lut[c & 15]);
			    output.push_back('>');
		    }
		    return output;
	    }
        
        inline std::string ConvertToHex(uint8_t byte)
        {
            char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            
            std::string str;
            str.append("0x");
            str.append(&hex[(byte & 0xF0) >> 4], 1);
            str.append(&hex[byte & 0xF], 1);
            return str;
        }
        
        inline std::string ConvertToHex(const uint8_t* pBytes, int size)
        {
            char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            
            std::string str;
            for (int i = 0; i < size; ++i) 
            {
                const uint8_t ch = pBytes[i];
                str.append("0x");
                str.append(&hex[(ch & 0xF0) >> 4], 1);
                str.append(&hex[ch & 0xF], 1);
                if( i < size - 1)
                    str.append(" ");
            }
            return str;
        }        
                
        inline bool LoadSettings(Settings& settings)
        {
            // For testing and debugging
            //nlohmann::json settingsjson = nlohmann::json::parse("{\"version\":\"1.0.0\",\"self_id\":0,\"units\":[{\"unit_type\":\"master\",\"unit_id\":0,\"unit_ipaddr\":\"192.168.1.1\"},{\"unit_type\":\"slave\",\"unit_id\":1,\"unit_ipaddr\":\"192.168.1.2\"}]}");
            int self_id_value, unit_id_value, index;
            std::string unit_type_value, unit_ipaddr_value;
            
            try
            { 
                // Load json file from current directory
                std::ifstream jsonFile("settings.json");
                // JSON object
                nlohmann::json settingsjson;
                // Deserialize json object from text content
                jsonFile >> settingsjson;
                
	            auto sitename = settingsjson.find("site_name");
	            if (sitename != settingsjson.end())
	            {
		            settings.SiteName = sitename->get<std::string>();
		            
		            std::string s;
		            
		            if (settings.SiteName.length() > 16)
		            {
			            s  = settings.SiteName.erase(16, settings.SiteName.length());			                
		            }
		            else
		            {
			            s = settings.SiteName;
		            }
			        
		            //Remove Illegal File Name Characters
		            std::replace(s.begin(), s.end(), ' ', '_'); 
		            std::replace(s.begin(), s.end(), '/', '_'); 
		            std::replace(s.begin(), s.end(), '\\', '_');
		            std::replace(s.begin(), s.end(), '*', '_');
		            std::replace(s.begin(), s.end(), '~', '_'); 
		            std::replace(s.begin(), s.end(), '?', '_'); 
		            std::replace(s.begin(), s.end(), '`', '_'); 
		            
		            settings.SiteName = s;
		            
		            std::cout << "SiteName: " << settings.SiteName << std::endl;
	            }
	            
                auto ver = settingsjson.find("version");
                if(ver != settingsjson.end())
                {
                    settings.Version = ver->get<std::string>();
                    std::cout << "Version: " << settings.Version << std::endl;
                }
                    
                auto self_id = settingsjson.find("self_id");
                if(self_id != settingsjson.end())
                {
                    self_id_value = self_id->get<int>();
                    std::cout << "self_id: " << self_id_value << std::endl;                    
                }
                
                auto gateway_ip = settingsjson.find("gateway_ipaddr");
                if(gateway_ip != settingsjson.end())
                {
                    settings.GatewayIPAddr = gateway_ip->get<std::string>();
                    std::cout << "gateway_ipaddr: " << settings.GatewayIPAddr << std::endl;
                }
	            
	            auto ntpserver_ip = settingsjson.find("ntpserver_ipaddr");
	            if (ntpserver_ip != settingsjson.end())
	            {
		            settings.NTPServerIPAddr = ntpserver_ip->get<std::string>();
		            std::cout << "ntpserver_ipaddr: " << settings.NTPServerIPAddr << std::endl;
	            }
	                            
                auto units = settingsjson.at("units");
                index = 0;
                for(auto it = units.begin(); it != units.end(); it++)
                {
                    std::cout << *it << std::endl;
                    
                    // Find needed elements in inner JSOM string
                    auto unit_type = it->find("unit_type");
                    if(unit_type == it->end())
                        continue;
                    unit_type_value = unit_type->get<std::string>();
                    
                    auto unit_id = it->find("unit_id");
                    if(unit_id == it->end())
                        continue;
                    unit_id_value = unit_id->get<int>();
                    
                    auto unit_ipaddr = it->find("unit_ipaddr");
                    if(unit_ipaddr == it->end())
                        continue;
                    unit_ipaddr_value = unit_ipaddr->get<std::string>();
                    
                    if(self_id_value == unit_id_value)  // Settings for the current board
                    {
                        settings.CurrentUnit.ID = self_id_value;
                        if(unit_type_value == "master")
                            settings.CurrentUnit.Type = UnitType::MASTER;
                        else if (unit_type_value == "submaster")
                            settings.CurrentUnit.Type = UnitType::SUBMASTER;
                        else if (unit_type_value == "slave")
                            settings.CurrentUnit.Type = UnitType::SLAVE;
                        else if (unit_type_value == "unused")
                            settings.CurrentUnit.Type = UnitType::UNUSED;
                        else
                            continue;
                        settings.CurrentUnit.IPAddress = unit_ipaddr_value;
                    }
                    else    // Settings for remote boards
                    {
                        settings.Units[index].ID = unit_id_value;
                        if(unit_type_value == "master")
                            settings.Units[index].Type = UnitType::MASTER;
                        else if (unit_type_value == "submaster")
                            settings.Units[index].Type = UnitType::SUBMASTER;
                        else if (unit_type_value == "slave")
                            settings.Units[index].Type = UnitType::SLAVE;
                        else if (unit_type_value == "unused")
                            settings.Units[index].Type = UnitType::UNUSED;
                        else
                            continue;
                        settings.Units[index].IPAddress = unit_ipaddr_value;
                        
                        // Move pointer to next slot in array
                        index++;
                    }
                    
                    if (index > (MAX_REMOTE_UNIT_NUMBERS - 1))
                        break;
                }
	            
	            /*
	            settingsjson["ntpserver_ipaddr"] = "0.0.0.1";	            	            
	            std::ofstream file("key.json");
	            file << settingsjson;
	            */
	            
            }
            catch(nlohmann::json::exception& ex)
            {
                std::cout << ex.what() << std::endl;
                return false;
            }
            catch(const std::exception& ex)
            {
                std::cout << ex.what() << std::endl;
                return false;
            }            
	        
	        
	        
            return true;
        }
        
    }   // namespace Utils
    
}   // namespace SafeEngineering


#endif // UTILITY_HPP
