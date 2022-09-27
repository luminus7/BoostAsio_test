#define _CRT_SECURE_NO_WARNINGS
#include<ctime>
#include<iostream>
#include<string>
#include<boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

int main() {
    try{
      boost::asio::io_service io_service;
//socket creation
      tcp::socket socket(io_service);
//take message from user input
        string command_user;
        cout << "Type your command: Start/Stop/Pause/Resume; Choose one of 4 commands at a time" << endl;
        cin >> command_user; //get command from the keyboard 
        cout << "Your command is: " << command_user << endl;
        string msg_index;
        if(command_user == "Start"){
             msg_index = "Start!n";
           }else if(command_user == "Stop"){
             msg_index = "Stop!n";
           }else if(command_user == "Pause"){
             msg_index = "Pause!n";
           }else{
             msg_index = "Resume!n";
          }
//connect socket
     socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 1234 ));
//take message from user input as the msg sent to server
     const string msg = msg_index;
     boost::system::error_code error;
//write msg to the buffer
     boost::asio::write( socket, boost::asio::buffer(msg), error );
     if( !error ) {
        cout << "Client sent message to Server!" << endl;
      }
     else {
        cout << "send failed: " << error.message() << endl;
     }
// getting response from server

//Repeatedly asking for user command after the server is started
    cout << "OK" << endl;
    while(1){
      cout << "Type your command: Start/Stop/Pause/Resume; Choose one of 4 commands at a time" << endl;
      cin >> command_user; //get command from the keyboard 
      cout << "Your command is: " << command_user << endl;
      if(command_user == "Start"){
            msg_index = "Start!n";
          }else if(command_user == "Stop"){
            msg_index = "Stop!n";
          }else if(command_user == "Pause"){
            msg_index = "Pause!n";
          }else{
            msg_index = "Resume!n";
        }
      boost::asio::write( socket, boost::asio::buffer(msg_index), error );
      if (msg_index == "Stop!n") //break out while loop after the client stop
        break;
    }

//     boost::asio::streambuf receive_buffer;
//     boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
//     if( error && error != boost::asio::error::eof ) {
//         cout << "receive failed: " << error.message() << endl;
//     }
// //read message from receive_buffer
//     else {
//         const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
//         cout << data << endl;
 //   }
    }
    catch(boost::system::system_error const& e) {
		std::cout << "Warning: could not connect : " << e.what() << std::endl;
	}
    return 0;
}