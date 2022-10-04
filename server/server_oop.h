#ifndef SERVER_H
#define SERVER_H

#ifdef __linux__ 
    //linux code goes here
#elif _WIN32
    // windows code goes here
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

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
    tcpServer(unsigned short port);

    void handleAccept(ptrTcpConnection &currentConnection, const boost::system::error_code& e);

    // tcpServer class deconstructor
    ~tcpServer();
};
/* end of tcpServer class */

#endif /* SERVER_H */