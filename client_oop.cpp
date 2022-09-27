#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

class tcpClient
{
private:
    // 기본적으로 사용되는 변수 선언
    // declare basic variables to be used
    tcp::resolver resolver;
    tcp::socket socket;
    boost::array<char, 1024> buf;
    boost::asio::streambuf request;
    std::ifstream sourceFile;

    // 서버에 세팅하는 Resolving 처리 함수
    // server setting (Resolving) manage function.
    void handleResolve(const boost::system::error_code& err, tcp::resolver::iterator myIterator) {
        if (!err) { // 오류가 발생하지 않은 경우
        // 리스트의 첫 번째 endpoint로 연결을 시도
        // 완전하게 연결이 성립될 때 까지 각각의 endpoint 연결 시도
            tcp::endpoint endpoint = *myIterator;
            socket.async_connect(
                endpoint,
                boost::bind(
                    &tcpClient::handleConnect,
                    this,
                    boost::asio::placeholders::error,
                    ++myIterator)
            );
        } else {    // 오류가 발생한 경우
            std::cout << "Error :: " << err.message() << "\n";
        }
    }

    // 실제로 연결을 실행하는 함수
    // function to handle actual connection
    void handleConnect(const boost::system::error_code& err, tcp::resolver::iterator myIterator) {
        // 오류가 발생하지 않은 경우 
        if (!err) { // if no error occured.
            // 연결이 성공적인 경우 요청을 보내게 됨
            // send request after successful connection
            boost::asio::async_write(
                socket,
                request,
                boost::bind(
                    &tcpClient::handleWriteFile,
                    this,
                    boost::asio::placeholders::error)
            );
        } else if (myIterator != tcp::resolver::iterator()) {
            // 연결에 실패한 경우 리스트의 다음 endpoint로 시도
            // if connection fails, try other endpoints in the list.
            socket.close();
            tcp::endpoint endpoint = *myIterator;
            socket.async_connect(
                endpoint,
                boost::bind(
                    &tcpClient::handleConnect,
                    this,
                    boost::asio::placeholders::error,
                    ++myIterator)
            );
        } else {
            std::cout << "Error : " << err.message() << "\n";
        }
    }

    // 실제로 파일을 쓰기 위한 함수
    // the function to write file (really).
    void handleWriteFile(const boost::system::error_code& err) {
        // 오류가 발생하지 않은 경우
        if (!err) { // if no error occured
            // 보내는 파일이 끝날 때 까지 버퍼에 담음
            if (sourceFile.eof() == false) {    // store into the buffer until reaching the end of the file.
                sourceFile.read(buf.c_array(), (std::streamsize) buf.size());
                
                if (sourceFile.gcount() <= 0) {
                    std::cout << "read file error" << std::endl;
                    return;
                }

                // 보내는 파일 정보
                std::cout << "send " << sourceFile.gcount() << "bytes, total:" << sourceFile.tellg() << "bytes.\n";

                // 실제로 파일을 전송
                // transfer the file to server.
                boost::asio::async_write(
                    socket,
                    boost::asio::buffer(buf.c_array(), sourceFile.gcount()),
                    boost::bind(
                        &tcpClient::handleWriteFile,
                        this,
                        boost::asio::placeholders::error)
                );

                // 오류가 발생한 경우
                if (err) {  // if the error occured while sending file.
                    std::cout << "send error : " << err << std::endl;
                    return;
                }
            }
            else{
                // every contents in the file has successfully send.
                return;
            }
        } else {
            std::cout << "Error : " << err.message() << "\n";
        }
    }

public:
    // tcpClient 클래스 생성자 정의
    // tcpClient class constructor
    tcpClient(boost::asio::io_service& io_service, const std::string& server, const std::string& path) : resolver(io_service), socket(io_service) {
        std::cout << __FUNCTION__ << " created." << std::endl;
        // 서버의 IP와 포트 주소를 나누어 저장
        // localhost:13
        size_t pos = server.find(':');
        if (pos == std::string::npos){
            std::cout << __FUNCTION__ << ": cannot find the server." << std::endl;
            return;
        }
        std::string portString = server.substr(pos + 1);    // 13
        std::string serverIP = server.substr(0, pos);   // localhost

        // 자신의 보내고 싶은 소스 파일을 열음
        // open the source we want to send.
        std::cout << __FUNCTION__ << ": open the file to send." << std::endl;
        sourceFile.open(path.c_str(), std::ios_base::binary | std::ios_base::ate);

        if (!sourceFile) {  // 소스 파일이 존재하지 않는 경우
            std::cout << __FUNCTION__ << ": failed to open the file." << path << std::endl;
            return;
        }

        // 파일에 대한 정보를 담는 변수
        // variable to store file informations.
        size_t fileSize = sourceFile.tellg();
        sourceFile.seekg(0);

        // 먼저 서버에 파일의 이름과 파일의 크기를 전송
        // first, transfer 'file name' and 'file size' to the server.
        std::ostream requestStream(&request);
        requestStream << path << "\n" << fileSize << "\n\n";
        std::cout << "request size : " << request.size() << std::endl;

        // 서버와 서비스의 이름을 번역하기 위해 비동기식으로 resolving 처리 (resolving endpoint host by using DNS)
        // endpoints의 리스트 처리
        tcp::resolver::query query(serverIP, portString);
        resolver.async_resolve(
            query,
            boost::bind(
                &tcpClient::handleResolve,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator)
        );
    }
};
/* end of tcpClient class */


int main(int argc, char* argv[]) {
    /** user input *
     * argv[1]: address of endpoint host, should be in the form of 'IP:PORT'
     * argv[2]: file path of the file to be transferred
     */
    try {
        if (argc != 3) {
            std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
            return 1;
        }

        std::string serverName = argv[1];
        std::string filePath = argv[2];     // "./transfer_file/test1.txt"

        // Create tcpClient object and run client program.
        boost::asio::io_service io_service;
        tcpClient myTcpClient(io_service, serverName, filePath);
        io_service.run();

        // 파일을 보낸 뒤에 보낸 파일 정보 출력
        // print out the file information after the file send ends.
        std::cout << "send file '" << filePath << "' completed successfully.\n";
    } catch (std::exception& e) { // 오류가 발생한 경우
        std::cerr << e.what() << std::endl;
    }

    // Terminate program.
    system("pause");
    return 0;
}