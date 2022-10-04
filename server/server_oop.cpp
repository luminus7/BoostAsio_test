#include "server_oop.h"

#include <iostream>
#include <string>
#include <fstream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>


// server port number
unsigned short port = 13;


tcpServer::tcpServer(unsigned short port)
	: /* public inheritance */ acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), true)
{
    // 서버 실행
	// run server by constructing tcpConnection class
	ptrTcpConnection newConnection(new tcpConnection(ioService));

	acceptor.async_accept(
		newConnection->socket(),
		boost::bind(
			&tcpServer::handleAccept,
			this,
			newConnection,
			boost::asio::placeholders::error)
	);
	ioService.run();
}

void tcpServer::handleAccept(ptrTcpConnection &currentConnection, const boost::system::error_code& e)
{
    std::cout << __FUNCTION__ << " " << e << ", " << e.message() << std::endl;
    if (!e) {
        currentConnection->start();
    }
    else {
        currentConnection.reset();
    }

    ptrTcpConnection newConnection(new tcpConnection(ioService));

    acceptor.async_accept(
        newConnection->socket(),
        boost::bind(
            &tcpServer::handleAccept,
            this,
            newConnection,
            boost::asio::placeholders::error)
    );
}

// tcpServer class deconstructor
tcpServer::~tcpServer()
{
    ioService.stop();
}

int main(int argc, char* argv[])
{
    /** possible user input *
     * argv[1]: port number of the server(endpoint host).
     */
    try {
        if (argc == 2) {
            port = atoi(argv[1]);
        }
        std::cout << argv[0] << " listen on port " << port << std::endl;

        // Create tcpServer object and run server program.
        tcpServer *myTcpServer = new tcpServer(port);

        std::cout << "ioService stopped" << std::endl;

        delete myTcpServer;
    } catch (std::exception& e) {   // 오류가 발생했을 경우
        std::cerr << e.what() << std::endl;
    }

    // Terminate program.
    system("pause");
    return 0;
}
