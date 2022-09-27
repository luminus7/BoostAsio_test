#include <iostream>
#include <string>
#include <fstream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

// server port number
unsigned short port = 13;

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

    // 파일을 읽어오는 요청 함수 다루기
    // Function to handle read request.  
    void handleReadRequest(const boost::system::error_code& err, std::size_t bytesTransferred) {
        if (err) {  // 오류가 발생한 경우
            return handleError(__FUNCTION__, err);
        }

        // 오류가 발생하지 않은 경우 파일 다운의 기본 설정 출력
        // print basic file information if no error occured. 
        std::cout << __FUNCTION__ << "(" << bytesTransferred << "B)"
            << ", in_avail = " << requestBuf.in_avail()
            << "B, size = " << requestBuf.size()
            << "B, max_size = " << requestBuf.max_size()
            << ".\n";

        // 파일에 대한 정보를 담을 변수
        // variable to store file informations.
        //  using streambuffer to receive file information 
        std::istream requestStream(&requestBuf);
        std::string filePath;
        requestStream >> filePath;
        requestStream >> fileSize;

        // 앞의 개행 문자 2개 제거
        // remove two new-line characters
        requestStream.read(buf.c_array(), 2);

        // 파일에 대한 정보 출력
        // print out file information
        std::cout << filePath << " size is " << fileSize << ", tellg = " << requestStream.tellg() << std::endl;
        size_t pos = filePath.find_last_of('\\');
        if (pos != std::string::npos){  // if not fail
            filePath = filePath.substr(pos + 1);
        }

        // store the file into 'received_file' directory
        std::string find_str = "transfer_file";
        filePath.replace(filePath.find("transfer_file"), find_str.length(), "received_file" );

        // 이제 내 컴퓨터에 출력할 파일을 열음
        // opening the file (include creating the file)
        outputFile.open(filePath.c_str(), std::ios_base::binary);

        // 출력 파일이 존재하지 않는 경우
        if (!outputFile) {  // if no file to be opened (error handling)
            std::cout << "failed to open" << filePath << std::endl;
            return;
        }

        // 파일의 여분 바이트를 써내려감
        // Writes the remaining bytes of the transmitted-byte-stream(requestStream) to a file(outputFile).
        // read unit(buf.size()) is 40KiB(40960Bytes).
        do {
            requestStream.read(buf.c_array(), (std::streamsize) buf.size());
            std::cout << __FUNCTION__ << " write " << requestStream.gcount() << "bytes.\n";
            // std::cout << "buf.size(): " << buf.size() << std::endl;
            outputFile.write(buf.c_array(), requestStream.gcount());
        } while (requestStream.gcount() > 0);

        // 버퍼에 정보를 읽어 옴
        // read file contents into the buffer asynchronously.
        async_read(
            mySocket,
            boost::asio::buffer(buf.c_array(), buf.size()),
            boost::bind(
                &tcpConnection::handleReadFileContent,
                shared_from_this(),     // get 'this'(the instance) as shared pointer.
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
            );
    }

    // 파일의 내용을 읽는 기능
    // read the file contents.
    void handleReadFileContent(const boost::system::error_code& err, std::size_t bytesTransferred) {
        // 바이트 전송
        // byte transfer (file contents including '\n' in the end of the file.)
        if (bytesTransferred > 0) {
            outputFile.write(buf.c_array(), (std::streamsize) bytesTransferred);
            std::cout << __FUNCTION__ << " recv " << outputFile.tellp() << "bytes." << std::endl;
            if (outputFile.tellp() >= (std::streamsize) fileSize) {
                return;
            }
        }

        // std::cout << "\tbuf.data(): " << (void*)( buf.data() ) << std::endl;
        std::cout << "\treceived data: " << buf.data();
        // std::cout << "\tbuf.c_array(): " << buf.c_array() << "!!";
        // const char *terry = "hello :)";
        // std::cout << "terry: " << terry << std::endl;

        if(err == boost::asio::error::eof) {
            // no problem
            std::cout << "boost::asio::error::eof" << std::endl;
            // waiting for another message..
            return;
        } else if (err) {  // 오류가 발생한 경우
            return handleError(__FUNCTION__, err);
        }

        // 버퍼에 정보를 읽어 옴
        // read data to the buffer
        async_read(
            mySocket,
            boost::asio::buffer(buf.c_array(), buf.size()),
            boost::bind(
                &tcpConnection::handleReadFileContent,
                shared_from_this(),     // get 'this'(the instance) as shared pointer.
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    }

    // 오류 처리 함수
    // Error handling function
    void handleError(const std::string& functionName, const boost::system::error_code& err) {
        std::cout << __FUNCTION__ << " in " << functionName << " due to " << err << " " << err.message() << std::endl;
    }

public:

    // tcpConnection 클래스의 생성자
    // tcpConnection class constructor
    tcpConnection(boost::asio::io_service& io_service)
        : /* public inheritance */ mySocket(io_service), fileSize(0) {}
        // initialize mySocket and fileSize

    // 실행 함수
    // Function to start tcpConnection.
    void start() {
        std::cout << __FUNCTION__ << " tcp connection."<< std::endl;
        // while statement in here?? *******************************************
        async_read_until(   // read until the message ends.
            mySocket,
            requestBuf,
            "\n\n",
            boost::bind(
                &tcpConnection::handleReadRequest,
                shared_from_this(),     // get 'this'(the instance) as shared pointer.
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
        );
    }

    // 현재의 소켓을 반환하는 함수
    // Function to return current socket
    boost::asio::ip::tcp::socket& socket() {
        return mySocket;
    }
};
/* end of tcpConnection class */


/* tcpServer class */
class tcpServer
    : private boost::noncopyable { /* singleton pattern */

private:
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::acceptor acceptor;

public:
    // tcpConnection 클래스의 포인터 변수 선언
    // ptr variable for tcpConnection class
    typedef boost::shared_ptr <tcpConnection> ptrTcpConnection;

    // 서버 생성자
    // tcpServer class constructor
    tcpServer(unsigned short port)
        : /* public inheritance */ acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), true) {
        
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

    void handleAccept(ptrTcpConnection currentConnection, const boost::system::error_code& e) {
        std::cout << __FUNCTION__ << " " << e << ", " << e.message() << std::endl;
        if (!e) {
            currentConnection->start();
        }
    }

    // tcpServer class deconstructor
    ~tcpServer() {
        ioService.stop();
    }
};
/* end of tcpServer class */

int main(int argc, char* argv[]) {
    /** possible user input *
     * argv[1]: port number of the server(endpoint host).
     */
    try {
        if (argc == 2) {
            port = atoi(argv[1]);
        }
        // while(1){
        //     std::cout << argv[0] << " listen on port " << port << std::endl;
        //     // Create tcpServer object and run server program.
        //     tcpServer *myTcpServer = new tcpServer(port);
        //     if(0) {  // server shutdown signal from client or error occured.
        //         delete myTcpServer;
        //     }
        // }
        std::cout << argv[0] << " listen on port " << port << std::endl;
    
        // Create tcpServer object and run server program.
        tcpServer *myTcpServer = new tcpServer(port);

        delete myTcpServer;
    } catch (std::exception& e) {   // 오류가 발생했을 경우
        std::cerr << e.what() << std::endl;
    }

    // Terminate program.
    system("pause");
    return 0;
}
