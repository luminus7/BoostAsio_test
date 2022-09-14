#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <unistd.h>	// getlogin_r()
#include <string>
#include <regex>

using boost::asio::ip::tcp;
using namespace std;

enum Option{
    START = 1,	// 1
    STOP,	// 2
    PAUSE,	// 3
    RESUME,	// 4
    OPTERR = -1
};

int fnChooseOption(){
    std::string choice;	// buffer to store chosen option.

    cout << "Please select running option" << endl;
    cout << "1) start" << endl;
    cout << "2) stop" << endl;
    cout << "3) pause" << endl;
    cout << "4) resume" << endl;
    /* basically Boost Asio library has an I/O service object */

    cout << "Enter option: " << endl;
    cin >> choice; 

    // using regular expression to check option
    regex re_start("[sS][tT][aA][rR][tT]");
    regex re_stop("[sS][tT][oO][pP]");
    regex re_pause("[pP][aA][uU][sS][eE]");
    regex re_resume("[rR][eE][sS][uU][mM][eE]");

    if( choice.compare("1")==0 || std::regex_match(choice, re_start) ){
    	cout << "selected option: start" << endl;
	return START;
    }else if( choice.compare("2")==0 || std::regex_match(choice, re_stop) ){
    	cout << "selected option: stop" << endl;
	return STOP;
    }else if( choice.compare("3")==0 || std::regex_match(choice, re_pause) ){
    	cout << "selected option: pause" << endl;
	return 3;
    }else if( choice.compare("4")==0 || std::regex_match(choice, re_resume) ){
    	cout << "selected option: resume" << endl;
	return RESUME;
    }else{
    	cout << "Please enter correct option.\n\n" << endl;
	return OPTERR;
    }

}

int main(){
    char username_client[100];

    cout << "client running on " << getlogin_r(username_client, 100) << endl;
    
    int is_valid_opt = 0;
    
    // iterate until valid option received
    do{
        is_valid_opt = fnChooseOption();
    }while( is_valid_opt == OPTERR ); 

    try {
        boost::asio::io_service io_service;
	
	// using resolver to convert domain name to TCP endpoint
	tcp::resolver resolver(io_service);
        
	// dest server> localhost, service> daytime protocol
        tcp::resolver::query query("localhost", "daytime");
	
	//get IP addr, Port info by through DNS
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	tcp::socket socket(io_service);				// initialize socket object
	boost::asio::connect(socket, endpoint_iterator);	// connect to server

	// read message from server
//	while(1){
//	    boost::array<char, 128> buf;	// declare buffer
//	    boost::system::error_code error;	// declare error managing variable
//
//	    // receive data from server & save the data in buffer
//	    size_t len_rd = socket.read_some(boost::asio::buffer(buf), error);
//	    if (error == boost::asio::error::eof){
//	        break;
//	    } else if(error) {
//	        throw boost::system::system_error(error);
//	    }
//	    cout.write(buf.data(), len_rd);	// print buffer data
//	}

	// write message to server & read message from server
	while(1){
	    boost::array<char, 128> buf_wr;	// declare write buffer
	    boost::array<char, 128> buf_rd;	// declare read buffer

	    boost::system::error_code error;	// declare error managing variable

	    buf_wr[0] = (char)(is_valid_opt + 48);	// convert int -> char & store to buffer
	    buf_wr[1] = '\0';

	    // send data to server
	    boost::asio::write(socket, boost::asio::buffer(buf_wr), error);
	    cout << "message sent to server >>>" << endl;

	    // receive data from server & save the data in buffer
	    size_t len_rd= socket.read_some(boost::asio::buffer(buf_rd), error);
	    if (error == boost::asio::error::eof){
	        break;
	    } else if(error) {
	        throw boost::system::system_error(error);
	    }

	    cout << "message received from server <<<" << endl;
	    cout << "received msg: ";
	    cout.write(buf_rd.data(), len_rd);	// print buffer data
	}
    } catch(exception & e) {
        cerr << e.what() << endl;
    }
    system("pause");
    return 0;
}
