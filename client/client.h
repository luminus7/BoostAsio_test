#pragma once
#include <fstream>

#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

class tcpClient
{
private:
    // declare basic variables to be used
    tcp::resolver resolver;
    tcp::socket socket;
    boost::array<char, 1024> buf;
    boost::asio::streambuf request;
    std::ifstream sourceFile;

    // server setting (Resolving) manage function.
    void handleResolve(const boost::system::error_code& err, tcp::resolver::iterator myIterator);

    // function to handle actual connection
    void handleConnect(const boost::system::error_code& err, tcp::resolver::iterator myIterator);

    // the function to write file (really).
    void handleWriteFile(const boost::system::error_code& err);

public:
    // tcpClient class constructor
    tcpClient(boost::asio::io_service& io_service, const std::string& server, const std::string& path);
};
/* end of tcpClient class */
