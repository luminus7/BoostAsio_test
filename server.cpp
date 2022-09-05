#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <unistd.h>	// getlogin_r()

using boost::asio::ip::tcp;
using namespace std;

// return date and time info of server machine
string make_daytime_string(){
    time_t now = time(0);
    return ctime(&now);
}

int main(){
    char username_Server[100];
    cout << "server running on " << getlogin_r(username_Server, 100) << endl;
    /* basically Boost Asio library has an I/O service object */
    try {
        boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 13));	// create socket manually, protocol> TCP, port> 13
	/* infinite loop for all connected clients */
	while(1){
	    tcp::socket socket(io_service);		// create socket obj
	    acceptor.accept(socket);			// and wait for connection
	    string message = make_daytime_string();	// create msg to be sent
	    boost::system::error_code ignored_error;	// send msg to the client
	    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
	}
    } catch(exception & e) {
        cerr << e.what() << '\n';
    }
    return 0;
}
