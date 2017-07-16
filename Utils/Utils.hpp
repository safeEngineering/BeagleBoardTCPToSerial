#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <iostream>

namespace SafeEngineering
{
    namespace Utils
    {
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
        
    }   // namespace Utils
    
}   // namespace SafeEngineering


#endif // UTILITY_HPP
