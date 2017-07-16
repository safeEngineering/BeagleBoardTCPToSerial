#include <iostream>
#include <ctime>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <asio.hpp>

#include "Serial.hpp"
#include "Tcp_Connection.hpp"


int main(int argc, char **argv)
{
    try
    {
        asio::io_service io;
        
        SafeEngineering::Comm::Serial com1(io);
        // Open UART connection
        com1.OpenSerial();
        
        asio::thread serialThread(boost::bind(&asio::io_service::run, &io));
        
        std::array<uint8_t, 8> msg;
        msg[0] = 0x01;
        msg[1] = 0x03;
        msg[2] = 0x63;
        msg[3] = 0x00;
        msg[4] = 0x00;
        msg[5] = 0x01;        
        msg[6] = 0x9A;
        msg[7] = 0x4E;

        char key;
        while ((key = getchar()) != 'q')
        {
            switch(key)
            {
                case 's':
                    com1.SendPacket(msg.data(), msg.size());
                    break;
            }
        }
        
        // Close UART connection
        com1.CloseSerial();
    }
    catch(std::exception& e)
    {
        std::cerr << "Main thrown exception: '" << e.what() << "'" <<std::endl;
    }
    
	return 0;
}
