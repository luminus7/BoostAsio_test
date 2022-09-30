#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define BOOST_USE_WINDOWS_H

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "tcp_connection.h"

/* tcpServer class */
class tcpServer
    : private boost::noncopyable { /* singleton pattern */

private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor;
    // server port number
    static const unsigned short port = 13;

public:
    // tcpConnection 클래스의 포인터 변수 선언
    // ptr variable for tcpConnection class
    typedef boost::shared_ptr <tcpConnection> ptrTcpConnection;

    // 서버 생성자
    // tcpServer class constructor
    tcpServer(unsigned short port, boost::asio::io_service& ioservice);

    void handleAccept(ptrTcpConnection &currentConnection, const boost::system::error_code& e);

    // tcpServer class deconstructor
    ~tcpServer();
};
/* end of tcpServer class */

