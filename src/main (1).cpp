#include <iostream>
#include <string>
#include <error.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <thread>

using namespace std;

bool readStable(int serialIo,char *data, int length)
{
    int total= 0;
    while(total!=length){
        int len = read(serialIo,&data[total],1);
        if (len <0){
            return false;
        }
        else {
            total+=len;
        }
    }
    return true;
}

void readSerial(int serialIo,int clientFd){
    char data;
    std::cout << "Waiting for serial";
    while (true){
        if (!readStable(serialIo,&data,1)){
            std::cout << "Serial unplugged?"<<std::endl;
            return;
        }
        if (write(clientFd,&data,1)!=1){
            std::cout << "Client Disconnected?"<<std::endl;
            return;
        }

    }
}

/**
 * @brief main
 * This is a very simple application.
 * It opens the serial device and starts listening on a tcp socket.
 * Then redirects all traffic to/from the network socket to the serial port.
 * @return
 */
int main()
{
    std::cout.rdbuf()->pubsetbuf( NULL, 0 );
    cout << "Hello World!" << endl;
    std::string serial = "/dev/ttyUSB0";

    int serialIo = open(serial.c_str(), O_RDWR | O_NOCTTY);
    if (serialIo == -1){
        std::cout << "No serial device detected" << std::endl;
        return -1;
    }
    struct termios tio;
    std::memset(&tio, 0,sizeof(tio));
    tio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_cc[VTIME]    = 1;
    tio.c_cc[VMIN]     = 0;
    tcflush(serialIo, TCIFLUSH);
    tcsetattr(serialIo, TCSANOW, &tio);

    u_int32_t size;

// I am going to comment this code out for now
#if 0 
    int serverFd, clientFd;


    struct sockaddr_in serv_addr, client_addr;


    serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if(serverFd < 0)
        printf("Socket create failed\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(2468);

    if (bind(serverFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cout << "Failed to bind" << std::endl;
        return 0;
    }

    listen(serverFd, 1);
#endif

    while(true){
        clientFd = accept(serverFd, (struct sockaddr *)&client_addr, &size);

        if (clientFd == -1){
            std::cout << "Failed accepting connection"<< std::endl;
        }
        else{
            std::cout << "Connected"<< std::endl;;
        }

        //now start another thread...
        std::thread background(readSerial,serialIo,clientFd);
        background.detach();

        uint32_t length;
        while (true){
            std::cout << "Waiting:"<< sizeof(uint32_t) << std::endl;;
            if (read(clientFd,&length, sizeof(uint32_t)) > 0){
                std::cout << "Reading:" << length << std::endl;
                if (length > 1024){
                    printf("Failed receiving\n");
                    close(clientFd);
                    break;
                }
                char buf[length];
                read(clientFd,&buf,length);
                write(serialIo,&buf,length);
            }

            else{
                printf("Failed receiving\n");
                close(clientFd);
                break;
            }
        }
    }
}

