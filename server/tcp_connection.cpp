#include "tcp_connection.h"

#include <filesystem> // for linux & windows

// tcpConnection 클래스의 생성자
// tcpConnection class constructor
tcpConnection::tcpConnection(boost::asio::io_service& io_service)
    : /* public inheritance */ mySocket(io_service), fileSize(0) {}
// initialize mySocket and fileSize


// 실행 함수
// Function to start tcpConnection.
void tcpConnection::start()
{
    std::cout << __FUNCTION__ << " tcp connection." << std::endl;
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
boost::asio::ip::tcp::socket& tcpConnection::socket()
{
    return mySocket;
}


// 파일을 읽어오는 요청 함수 다루기
// Function to handle read request.  
void tcpConnection::handleReadRequest(const boost::system::error_code& err, std::size_t bytesTransferred)
{
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

	//std::cout << (const char *) requestBuf.data().data() << std::endl;


	// 파일에 대한 정보를 담을 변수
	// variable to store file informations.
	//  using streambuffer to receive file information 
	std::istream requestStream(&requestBuf);
	std::string filePath;
	std::string fileName;
	requestStream >> filePath;
	requestStream >> fileSize;

	// 앞의 개행 문자 2개 제거
	// remove two new-line characters
	requestStream.read(buf.c_array(), 2);

	// 파일에 대한 정보 출력
	// print out file information
	std::cout << filePath << " size is " << fileSize << ", tellg = " << requestStream.tellg() << std::endl;
	size_t pos = filePath.find_last_of(std::filesystem::path::preferred_separator);
	if (pos != std::string::npos) {  // if not fail
		fileName = filePath.substr(pos + 1);
	}
	
	std::cout << filePath << std::endl;

	// store the file into 'received_file' directory
	std::string find_str = "transfer_file";
	filePath = filePath.replace(filePath.find(find_str), find_str.length(), "received_file");

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
		requestStream.read(buf.c_array(), (std::streamsize)buf.size());
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
void tcpConnection::handleReadFileContent(const boost::system::error_code& err, std::size_t bytesTransferred)
{
	// 바이트 전송
	// byte transfer (file contents including '\n' in the end of the file.)
	if (bytesTransferred > 0) {
		outputFile.write(buf.c_array(), (std::streamsize)bytesTransferred);
		std::cout << __FUNCTION__ << " recv " << outputFile.tellp() << "bytes." << std::endl;
		if (outputFile.tellp() >= (std::streamsize)fileSize) {
			return;
		}
	}

	// std::cout << "\tbuf.data(): " << (void*)( buf.data() ) << std::endl;
	std::cout << "\treceived data: " << buf.data();
	// std::cout << "\tbuf.c_array(): " << buf.c_array() << "!!";
	// const char *terry = "hello :)";
	// std::cout << "terry: " << terry << std::endl;

	if (err == boost::asio::error::eof) {
		// no problem
		std::cout << "boost::asio::error::eof" << std::endl;
		// waiting for another message..
		return;
	}
	else if (err) {  // 오류가 발생한 경우
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
void tcpConnection::handleError(const std::string& functionName, const boost::system::error_code& err)
{
    std::cout << __FUNCTION__ << " in " << functionName << " due to " << err << " " << err.message() << std::endl;
}

