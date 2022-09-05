#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <unistd.h>	// getlogin_r()

using boost::asio::ip::tcp;
using namespace std;

int main(){
    char username_Client[100];
    cout << "client running on " << getlogin_r(username_Client, 100) << endl;
    /* basically Boost Asio library has an I/O service object */
    try {
        boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);					// using resolver to convert domain name to TCP endpoint
        tcp::resolver::query query("localhost", "daytime");			// dest server> localhost, service> daytime protocol
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);	//get IP addr, Port info by through DNS
	tcp::socket socket(io_service);						// initialize socket object
	boost::asio::connect(socket, endpoint_iterator);			// connect to server

	while(1){
	    boost::array<char, 128> buf;	// declare buffer
	    boost::system::error_code error;	// declare error managing var
	    size_t len = socket.read_some(boost::asio::buffer(buf), error);	// receive data from server & save the data in buffer
	    if (error == boost::asio::error::eof){
	        break;
	    } else if(error) {
	        throw boost::system::system_error(error);
	    }
	    cout.write(buf.data(), len);	// print buffer data
	}
    } catch(exception & e) {
        cerr << e.what() << endl;
    }
    system("pause");
    return 0;
}
