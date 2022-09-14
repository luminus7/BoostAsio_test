#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <unistd.h>	// getlogin_r()

using boost::asio::ip::tcp;
using namespace std;

// return date and time info of server machine
string make_daytime_string(){
    time_t now = time(0);
    return ctime(&now);
}

enum Option{
    START = 1,	// 1
    STOP,	// 2
    PAUSE,	// 3
    RESUME,	// 4
    OPTERR = -1
};

string interpretOption(int recv_opt){
    std::string opt_name;
    switch (recv_opt){
    	case START:
	    opt_name = "START";
	    break;
    	case STOP:
	    opt_name = "STOP";
	    break;
    	case PAUSE:
	    opt_name = "PAUSE";
	    break;
    	case RESUME:
	    opt_name = "RESUME";
	    break;
    }
    return opt_name;
}

int main(){
    char username_Server[100];
    cout << "server running on " << getlogin_r(username_Server, 100) << endl;
    /* basically Boost Asio library has an I/O service object */
    try {
        boost::asio::io_service io_service;
	
	// create socket manually, protocol> TCP, port> 13
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 13));
	
	// send message to connected client
	/* infinite loop for all connected clients */
//	while(1){
//	    tcp::socket socket(io_service);		// create socket obj
//	    acceptor.accept(socket);			// and wait for connection
//	    string message = make_daytime_string();	// create msg to be sent
//	    boost::system::error_code ignored_error;	// send msg to the client
//	    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
//	}

	// receive message from connected client
	/* infinite loop for all connected clients */
	while(1){
	    boost::array<char, 128> buf_rd;		// declare read buffer (read from client)
	    boost::array<char, 128> buf_wr;		// declare write buffer (write to client)
	    
	    boost::system::error_code error;
	    boost::system::error_code ignored_error;

	    tcp::socket socket(io_service);		// create socket obj
	    acceptor.accept(socket);			// and wait for connection
	    size_t len_rd = socket.read_some(boost::asio::buffer(buf_rd), error);	// read data from client
	    cout << "message received from client <<<" << endl;
	    // Currently, buffer have garbage data as well due to the buffer size.
	    //cout.write(buf_rd.data(), len_rd);		// print read buffer data

            string message_recv = interpretOption( atoi(buf_rd.data()) );	// convert data to interpret
            //cout << "msg converting test: " << atoi(buf_rd.data()) << endl;
	    cout << "received msg: " << message_recv << endl;

	    string message_snd = make_daytime_string();	// create msg to be sent
	    boost::asio::write(socket, boost::asio::buffer(message_snd), error);	// send msg to the client
	    cout << "message sent to client >>>" << endl;
	}
    } catch(exception & e) {
        cerr << e.what() << '\n';
    }
    return 0;
}
