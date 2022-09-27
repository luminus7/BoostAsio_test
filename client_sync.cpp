#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <unistd.h>	// getlogin_r()
#include <string>
#include <regex>

#include <fstream>  // file I/O
#include <vector>
#include <sys/stat.h>

using boost::asio::ip::tcp;
using namespace std;

enum OPTION{
    OPT_START = 1,	// 1
    OPT_STOP,	// 2
    OPT_PAUSE,	// 3
    OPT_RESUME,	// 4
    OPT_ERR = -1
};

enum FILE_STATUS{
    FILE_EXIST = 1,	// 1
    FILE_ABORT,	    // 2
    FILE_ERR = -1
};

int _fnCheckOption(string choice_){
    // using regular expression to check option
    regex regex_start("[sS][tT][aA][rR][tT]");
    regex regex_stop("[sS][tT][oO][pP]");
    regex regex_pause("[pP][aA][uU][sS][eE]");
    regex regex_resume("[rR][eE][sS][uU][mM][eE]");

    if( choice_.compare("1")==0 || std::regex_match(choice_, regex_start) ){
    	std::cout << "selected option: start" << endl;
	    return OPT_START;
    }else if( choice_.compare("2")==0 || std::regex_match(choice_, regex_stop) ){
    	std::cout << "selected option: stop" << endl;
	    return OPT_STOP;
    }else if( choice_.compare("3")==0 || std::regex_match(choice_, regex_pause) ){
    	std::cout << "selected option: pause" << endl;
	    return OPT_PAUSE;
    }else if( choice_.compare("4")==0 || std::regex_match(choice_, regex_resume) ){
    	std::cout << "selected option: resume" << endl;
	    return OPT_RESUME;
    }else{
    	std::cout << "** Please enter correct option. **\n\n" << endl;
	    return OPT_ERR;
    }

    // undesired behavior (possibly error.)
    return OPT_ERR;
}

int fnChooseOption(){
    std::string choice;	    // buffer to store chosen option.
    int is_valid_opt = 0;

    do{
        std::cout << "Please select running option" << endl;
        std::cout << "1) start" << endl;
        std::cout << "2) stop" << endl;
        std::cout << "3) pause" << endl;
        std::cout << "4) resume" << endl;
        std::cout << "Enter option: ";
        std::cin >> choice; 
   
        is_valid_opt = _fnCheckOption(choice);
    }while( is_valid_opt == OPT_ERR ); 

    // return valid option
    return is_valid_opt;
}

int _fnCheckfile(string file_name){

    // check accessibility of the file.
    // possible replacement for non-POSIX system: fopen(), fclose()
    struct stat buffer;
    file_name = "./transfer_file/" + file_name;

    // using regular expression to check option
    regex regex_test("[tT][eE][sS][tT]d*\\.txt");
    regex regex_abort("[aA][bB][oO][rR][tT]");

    if( stat(file_name.c_str(), &buffer) == 0 ){
        std::cout << "\tthe file exist!" << endl;
        return FILE_EXIST;   // exist
    } else if( std::regex_match(file_name, regex_abort) ){
        std::cout << "\t** file transfer aborted while selecting file. **" << endl;
        return FILE_ABORT;   // abort
    } else{
        std::cout << "\t** Please enter correct file name. **\n\n" << endl;
        return FILE_ERR;  // error
    }

    // undesired behavior (possibly error.)
    return FILE_ERR;
}

/**
 * fnChooseFile()
 * receive file name and check validity.
 * return file_name 
 */
string fnChooseFile(){
    std::string file_name;	// buffer to store chosen file.
    int is_valid = 0;
    
    // iterate until valid file name received
    do{
        std::cout << "\tPlease select file to be sent to server" << endl;
        std::cout << "\t+-------------- sample ---------------+" << endl;
        std::cout << "\t|              test1.txt              |" << endl;
        std::cout << "\t+-------------------------------------+" << endl;
        std::cout << "\tJust type in 'abort' if you don't want to send file." << endl;

        std::cout << "\tEnter file name: ";
        std::cin >> file_name; 

        is_valid = _fnCheckfile(file_name);
        if(is_valid == FILE_ABORT) { // abort
            return "aborted";
        } else if (is_valid == FILE_ERR){   // file error
            continue;
        } else if (is_valid == FILE_EXIST){ // file exist
            return file_name;
        }
    }while(1);

    // undesired behavior (possibly error.)
    // return "\t*ERROR: undesired behavior while choosing file*";
}

bool readFileBytes(std::string filePath, unsigned char *_data, int *_datalen)
{
	std::ifstream is(filePath, std::ifstream::binary);
	if (is) {
		// extract size of the file (seekg)
		is.seekg(0, is.end);
		int length = (int)is.tellg();
		is.seekg(0, is.beg);

		// initialize buffer with malloc to store data
		unsigned char * buffer = (unsigned char*)malloc(length);

		// read data in the size of buffer block:
		is.read((char*)buffer, length);
		is.close();
		_data = buffer; 
        cout << (void*)_data << endl;
		*_datalen = length;
	}
    
	return true;
}

// https://www.boost.org/doc/libs/1_80_0/doc/html/boost_asio/reference/async_write/overload1.html
// async_write handler (call back function?)
 void async_write_handler(
  const boost::system::error_code& error, // Result of operation.
  std::size_t bytes_transferred           // Number of bytes written from the
                                          // buffers. If an error occurred,
                                          // this will be less than the sum
                                          // of the buffer sizes.
){std::cout << "async_write_handler returned" << endl;};


int main(){
    char username_client[100];

    std::cout << "client running on " << getlogin_r(username_client, 100) << endl;
    /* basically Boost Asio library has an I/O service object */
    /* but 'io_service' is now depricated it changed into 'io_context' */

    int option = 0;
    string file_name = "";      // buffer to store file name
    unsigned char* file_data;  // buffer to store file data
    int* file_len;              // buffer to store file length
    int is_file_transfer = 0;   // file transfer flag

    // iterate until valid option received
    option = fnChooseOption();
    file_name = fnChooseFile();
    if(file_name == "aborted"){
        is_file_transfer = 0;
    } else {
        is_file_transfer = 1;
        std::cout << "\tfile name is '" << file_name << "'" <<endl;
    }

    // convert file data into rvalue(std::vector<char>)
//    auto file_in_bytes = move( readFileBytes(file_name) );
    if( readFileBytes(file_name, file_data, file_len) ){
        is_file_transfer = 1;
    } else {    // the file has no data in it.
        is_file_transfer = 0;
    }

    cout << "\tstore 'file_data'" << endl;
    auto file_in_bytes = file_data;
    cout << "\tafter storing 'file_data'" << endl;

    try {
        boost::asio::io_service io_service; // io_service[depricated] -> io_context
	
	    // using resolver to convert domain name to TCP endpoint
	    tcp::resolver resolver(io_service);
        
	    // dest server> localhost, service> daytime protocol
        tcp::resolver::query query("localhost", "daytime");
	
	    //get IP addr, Port info by through DNS
	    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	    tcp::socket socket_(io_service);				// initialize socket object
	    boost::asio::connect(socket_, endpoint_iterator);	// connect to server

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

            buf_wr[0] = (char)(option + 48);	// convert int -> char & store to buffer
            buf_wr[1] = '\0';

            // send data to server (synchronous)
            // boost::asio::write(socket_, boost::asio::buffer(buf_wr), error);
            // cout << "message sent to server >>>" << endl;
            
            // send option data to server (asynchronous)
            // boost::asio::async_write(
            //     socket_,
            //     // boost::asio::buffer(request),
            //     boost::asio::buffer(&buf_wr, 100UL),
            //     async_write_handler   //write handler (call back function) // possible solutions:: 1. bind function 2. by using strand
            //     //boost::bind()
            // );
            // std::cout << "message[option] sent to server >>>" << endl;

            // send file data to server (asynchronous)
            /********/
            // if(is_file_transfer == 1){
            //     boost::asio::async_write(
            //         socket_,
            //         // boost::asio::buffer(request),
            //         boost::asio::buffer(&file_in_bytes, 1000UL),
            //         async_write_handler   //write handler (call back function) // possible solutions:: 1. bind function 2. by using strand
            //         //boost::bind()
            //     );
            //     std::cout << "message[file] sent to server >>>" << endl;
            // }

            // receive data from server & save the data in buffer
            size_t len_rd= socket_.read_some(boost::asio::buffer(buf_rd), error);
            if (error == boost::asio::error::eof){
                break;
            } else if(error) {
                throw boost::system::system_error(error);
            }

            std::cout << "message received from server <<<" << endl;
            std::cout << "received msg: ";
            std::cout.write(buf_rd.data(), len_rd);	// print buffer data
        }
    } catch(exception & e) {
        cerr << e.what() << endl;
    }
    system("pause");
    return 0;
}
