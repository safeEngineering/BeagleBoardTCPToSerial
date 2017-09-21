#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <iostream>
#include <fstream>
#include <array>
#include <chrono>

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
            // Parameters of the remote boards in the system
            std::array<Unit, MAX_REMOTE_UNIT_NUMBERS> Units;
        } Settings;
	    
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
