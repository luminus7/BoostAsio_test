#pragma once

#include <fstream>

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

// 실질적인 통신을 담당하는 tcpConnection 클래스 정의
/**
 * tcpConnection class
 *
 * This class is responsible for communication between server & client.
 * */

class tcpConnection
    : public boost::enable_shared_from_this <tcpConnection>
{

    // 기본적인 변수들을 선언
    // declare basic variables
private:
    boost::asio::streambuf requestBuf;
    boost::asio::ip::tcp::socket mySocket;
    boost::array<char, 40960> buf;
    size_t fileSize;
    std::ofstream outputFile;

    // Function to handle read request.  
    void handleReadRequest(const boost::system::error_code& err, std::size_t bytesTransferred);

    // read the file contents.
    void handleReadFileContent(const boost::system::error_code& err, std::size_t bytesTransferred);

    // Error handling function
    void handleError(const std::string& functionName, const boost::system::error_code& err);

public:
    // tcpConnection class constructor
    tcpConnection(boost::asio::io_service& io_service);

    // Function to start tcpConnection.
    void start();

    // Function to return current socket
    boost::asio::ip::tcp::socket& socket();
};
/* end of tcpConnection class */