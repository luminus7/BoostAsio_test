#define _CRT_SECURE_NO_WARNINGS
#include<ctime>
#include<iostream>
#include<string>
#include<boost/asio.hpp>
#include <unistd.h> 

using boost::asio::ip::tcp;
using namespace std;

string read_(tcp::socket & socket) {
       boost::asio::streambuf buf;
       boost::system::error_code error;
       boost::asio::read_until( socket, buf, "n", error );
       if (error){
              // cout << error << endl;
              return ("");
       }
       string data = boost::asio::buffer_cast<const char*>(buf.data());
       return data;
}
void send_(tcp::socket & socket, const string& message) {
       const string msg = message + "n";
       boost::asio::write( socket, boost::asio::buffer(message) );
}
// string make_daytime_string(){
//     time_t now = time(0);
//     return ctime(&now);
// }
void test_function(){
                sleep(3);
                cout << "running test_function" << endl;
}

int main() {
    try{
       boost::asio::io_service io_service;
//listen for new connection
      tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), 1234 ));
      
            //socket creation 
             tcp::socket socket_(io_service);
             acceptor_.accept(socket_);
             socket_.non_blocking(true);
       while (1){
              string message = read_(socket_);
              cout << message << endl;
              if (message == "Start!n"){
                 //write operation with Hello message from Client
                  send_(socket_, "You are running Start command");
                  while(1){
                     // cout << "running test_function" << endl;
                     // sleep(3);
                     test_function();
                     message = read_(socket_);
                     if (message == "Stop!n"){
                            cout << message << endl;
                            send_(socket_, "You are running Stop command");
                            return(0);
                     }
                     if (message == "Pause!n"){  //Use socket blocking mode, the program is block by read_(socket) function
                            cout << message << endl;
                            send_(socket_, "You are running Pause command");
                            socket_.non_blocking(false);
                            message = read_(socket_);
                     }
                     if (message == "Resume!n"){ //Is this resume? Changing the socket to non-blocking mode to make the program continue
                            cout << message << endl;
                            send_(socket_, "You are running Resume command");
                            socket_.non_blocking(true);
                     }
                  }
              }else{
                     cout << "Wrong command" << endl;
              }
       }
}catch(boost::system::system_error const& e) {
		std::cout << "Warning: could not connect : " << e.what() << std::endl;
	}
   return 0;
}