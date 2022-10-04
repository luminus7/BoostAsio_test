#include "tcp_connection.h"

#include <filesystem> // for linux & windows

// tcpConnection Ŭ������ ������
// tcpConnection class constructor
tcpConnection::tcpConnection(boost::asio::io_service& io_service)
    : /* public inheritance */ mySocket(io_service), fileSize(0) {}
// initialize mySocket and fileSize


// ���� �Լ�
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


// ������ ������ ��ȯ�ϴ� �Լ�
// Function to return current socket
boost::asio::ip::tcp::socket& tcpConnection::socket()
{
    return mySocket;
}


// ������ �о���� ��û �Լ� �ٷ��
// Function to handle read request.  
void tcpConnection::handleReadRequest(const boost::system::error_code& err, std::size_t bytesTransferred)
{
	if (err) {  // ������ �߻��� ���
		return handleError(__FUNCTION__, err);
	}

	// ������ �߻����� ���� ��� ���� �ٿ��� �⺻ ���� ���
	// print basic file information if no error occured. 
	std::cout << __FUNCTION__ << "(" << bytesTransferred << "B)"
		<< ", in_avail = " << requestBuf.in_avail()
		<< "B, size = " << requestBuf.size()
		<< "B, max_size = " << requestBuf.max_size()
		<< ".\n";

	//std::cout << (const char *) requestBuf.data().data() << std::endl;


	// ���Ͽ� ���� ������ ���� ����
	// variable to store file informations.
	//  using streambuffer to receive file information 
	std::istream requestStream(&requestBuf);
	std::string filePath;
	std::string fileName;
	requestStream >> filePath;
	requestStream >> fileSize;

	// ���� ���� ���� 2�� ����
	// remove two new-line characters
	requestStream.read(buf.c_array(), 2);

	// ���Ͽ� ���� ���� ���
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

	// ���� �� ��ǻ�Ϳ� ����� ������ ����
	// opening the file (include creating the file)
	outputFile.open(filePath.c_str(), std::ios_base::binary);

	// ��� ������ �������� �ʴ� ���
	if (!outputFile) {  // if no file to be opened (error handling)
		std::cout << "failed to open" << filePath << std::endl;
		return;
	}

	// ������ ���� ����Ʈ�� �᳻����
	// Writes the remaining bytes of the transmitted-byte-stream(requestStream) to a file(outputFile).
	// read unit(buf.size()) is 40KiB(40960Bytes).
	do {
		requestStream.read(buf.c_array(), (std::streamsize)buf.size());
		std::cout << __FUNCTION__ << " write " << requestStream.gcount() << "bytes.\n";
		// std::cout << "buf.size(): " << buf.size() << std::endl;
		outputFile.write(buf.c_array(), requestStream.gcount());
	} while (requestStream.gcount() > 0);

	// ���ۿ� ������ �о� ��
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

// ������ ������ �д� ���
// read the file contents.
void tcpConnection::handleReadFileContent(const boost::system::error_code& err, std::size_t bytesTransferred)
{
	// ����Ʈ ����
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
	else if (err) {  // ������ �߻��� ���
		return handleError(__FUNCTION__, err);
	}

	// ���ۿ� ������ �о� ��
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

// ���� ó�� �Լ�
// Error handling function
void tcpConnection::handleError(const std::string& functionName, const boost::system::error_code& err)
{
    std::cout << __FUNCTION__ << " in " << functionName << " due to " << err << " " << err.message() << std::endl;
}

