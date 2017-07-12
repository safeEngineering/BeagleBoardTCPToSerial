//============================================================================
// Name        : Test2.cpp
// Author      : AE
// Version     :
// Copyright   : Your copyright notice
// Description : Sample Program to Use ASIO Serial Port Library
//============================================================================

#include <iostream>
using namespace std;
#include <string>
#include <asio.hpp>

#define MAXLEN 512 // maximum buffer size

static ::string string_to_hex(const std::string& input)
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

void SerialTest()
{
	asio::io_service io;

	try
	{
	    // create a serial port object
		asio::serial_port serial(io);

		// open the platform specific device name
		// windows will be COM ports, linux will use /dev/ttyS* or /dev/ttyUSB*, etc
		serial.open("/dev/ttyO1");

		for (;;)
		{
		    // get a string from the user, sentiel is exit
			std::string input;
			std::cout << "Enter Message: ";
			std::cin >> input;

			if (input == "exit") break;

	        // write to the port
			// asio::write guarantees that the entire buffer is written to the serial port
			asio::write(serial, asio::buffer(input));

			char data[MAXLEN];

			// read bytes from the serial port
			// asio::read will read bytes until the buffer is filled
			size_t nread = asio::read(
			    serial,
				asio::buffer(data, input.length()));

			std::string message(data, nread);
			
			std::string hexMessage = string_to_hex(message);

			std::cout << "Recieved: ";
			std::cout << message << "     ";
			std::cout << hexMessage << std::endl;
			
		}

		serial.close();
	}
	catch (asio::system_error& e)
	{
		std::cerr << e.what() << std::endl;
	}
}


int main() {
	
	cout << "Ensure BeagleBoard Serial Port tty01 TX and Rx Lines are wired together and then type some text and press enter and it should be echoed back. Type 'exit' to stop the program" << endl; 
	
	SerialTest();
	
	return 0;
	
}


