# 2022 Research project - cpp Boost.Asio library test #


## How to download project ##
```bash
$ git clone https://github.com/luminus7/BoostAsio_test.git 
$ cd BoostAsio_test
```

## Installation & Run [Linux] ##
1. update your apt-get
```bash
 $ sudo apt-get update
 ```

2. download Boost.Asio library [1.80.0 version]
```bash
 $ wget -c https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
 $ tar -xvf boost_1_80_0.tar.gz
 $ cd boost_1_80_0
 $ ./bootstrap.sh
 ```

3. install Boost.Asio library
```bash
 $ ./bootstrap.sh
 ```

4. make configuration file to build Boost.Asio library
```bash
 $ sudo ./b2 install
 ```

5. compile source codes
we need g++ compiler includes c++11 or more.
```bash
 $ g++ -o server.run server_oop.cpp -lpthread
 $ g++ -o client.run client_oop.cpp -lpthread
 ```

6. run executable files in local host
you can modify the code as you wish.
```bash
 $ sudo ./server.run <PORT>
 $ sudo ./client.run <DEST_IP:DEST_PORT> <FILE_PATH>
 ```

 7. information about server_oop.cpp & client_oop.cpp
[2022.09.27] Current implementation can transfer the file from client -(to)-> server in asynchronous way.

## Installation guide reference ##
[Korean guide - blog](https://int-i.github.io/cpp/2020-09-19/ubuntu-boost/)

