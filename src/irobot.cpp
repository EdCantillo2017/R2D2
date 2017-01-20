#include "irobot.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>


iRobot::iRobot()
{
}

error iRobot::start(std::string connection)
{
    serialIo = open(connection.c_str(), O_RDWR | O_NOCTTY);
    if (serialIo == -1){
        return ERROR::NOSUCHDEVICE;
    }
    struct termios tio;
    std::memset(&tio, 0,sizeof(tio));
    tio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_cc[VTIME]    = 1;
    tio.c_cc[VMIN]     = 0;
    tcflush(serialIo, TCIFLUSH);
    tcsetattr(serialIo, TCSANOW, &tio);
    isNetwork = false;

    return writeData(rData(OPCODE::start));

}

error iRobot::startNetwork(std::string host,uint16_t port)
{
    struct sockaddr_in serveraddr;
    struct hostent *server;

    serialIo = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname(host.c_str());
    if (server == 0) {
        std::cout << "ERROR, no such host as "<< host;
        return ERROR::NOSUCHDEVICE;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);

    if (connect(serialIo, (const sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        std::cout << "Can't connect' "<< host;
        return ERROR::NOSUCHDEVICE;
    }
    isNetwork = true;
    return writeData(rData(OPCODE::start));

}



error iRobot::reset()
{
    if (writeData(rData(OPCODE::reset))==ERROR::NONE){
        disconnect();
        return ERROR::NONE;
    }
    return ERROR::NOSUCHDEVICE;
}

error iRobot::stop()
{
    if (writeData(rData(OPCODE::stop))==ERROR::NONE){
        disconnect();
        return ERROR::NONE;
    }
    return ERROR::NOSUCHDEVICE;
}

error iRobot::baud(byte dataRate)
{
    if (writeData(pbaud(dataRate))==ERROR::NONE){
        struct termios tio;
        std::memset(&tio, 0,sizeof(tio));
        tio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
        tio.c_iflag = IGNPAR;
        tio.c_cc[VTIME]    = 1;
        tio.c_cc[VMIN]     = 0;
        tcflush(serialIo, TCIFLUSH);
        tcsetattr(serialIo, TCSANOW, &tio);
        return ERROR::NONE;
    }
    return ERROR::NOTREADY;
}


error iRobot::modesafe()
{
    return writeData(rData(OPCODE::safe));
}

error iRobot::modefull()
{
    return writeData(rData(OPCODE::full));
}

error iRobot::clean()
{
    return writeData(rData(OPCODE::clean));
}

error iRobot::max()
{
    return writeData(rData(OPCODE::max));
}

error iRobot::spot()
{
    return writeData(rData(OPCODE::spot));
}

error iRobot::seekDock()
{
    return writeData(rData(OPCODE::seekDock));
}

error iRobot::power()
{
    return writeData(rData(OPCODE::power));
}

error iRobot::schedule(const pSchedule &data)
{
    return writeData(data);
}

error iRobot::settime(const pDate &data)
{
    return writeData(data);
}

error iRobot::drive(uint16_t velocity, uint16_t radius)
{
    return writeData(pDrive(velocity,radius,OPCODE::drive));
}

error iRobot::dStraight(uint16_t velocity)
{
    return writeData(pDrive(velocity,pDrive::STRAIGHT,OPCODE::drive));
}

error iRobot::dClockwise(uint16_t velocity)
{
    return writeData(pDrive(velocity,pDrive::CW,OPCODE::drive));
}

error iRobot::dCClockwise(uint16_t velocity)
{
    return writeData(pDrive(velocity,pDrive::CCW,OPCODE::drive));
}

error iRobot::driveDirect(uint16_t rightVelocity, uint16_t leftVelocity)
{
    return writeData(pDrive(rightVelocity,leftVelocity,OPCODE::driveDirect));
}

error iRobot::drivePWN(uint16_t rightPWM, uint16_t leftPWM)
{
    return writeData(pDrive(rightPWM,leftPWM,OPCODE::drivePWM));
}

error iRobot::song(const pSong &data)
{
    return writeData(data);
}

error iRobot::play(uint8_t songNo)
{
    return writeData(pPlay(songNo));
}

error iRobot::sensors(uint8_t sensor)
{
    return writeData(pSensorRequest(sensor));
}

error iRobot::squerylist(std::initializer_list<uint8_t> list)
{
    return writeData(pQuery(list));
}

error iRobot::sstream(std::initializer_list<uint8_t> list)
{
    return writeData(pStream(list));
}

error iRobot::spause()
{
    return writeData(pStrCtl(true));
}

error iRobot::sresume()
{
    return writeData(pStrCtl(false));
}

error iRobot::sensorStart()
{
    std::function<void()> read =
            [this](){
        std::shared_ptr<pSensor> pktIn;

        while (true){
            pktIn = readData();
            if (!pktIn){
                continue;
            }

            if (pktIn->getLength()==-1){
                //special packet in packet...
                pktIn->streamToMe(this);
                continue;
            }

            auto result = callbacks.find(pktIn->getType());
            if (result==callbacks.end()){
                std::cout<<"No callback registered for packet type" << pktIn->getType()<<std::endl;
            }
            else {
                result->second(pktIn);
            }

        }

    };

    readThread = std::thread(read);
    readThread.detach();
    return ERROR::NONE;
}

void iRobot::registerCallback(std::initializer_list<byte> sensors, std::function<void (std::shared_ptr<pSensor>)> function)
{
    for (byte sensor:sensors){
        auto ptr = callbacks.find(sensor);
        if (ptr!=callbacks.end()){
            callbacks.erase(ptr);
        }
        callbacks.insert({{sensor,function}});
    }
}




error iRobot::writeData(const rData &data)
{
    if (isNetwork){
        uint32_t length = 1+data.dataLen;
        write(serialIo,&length,sizeof(uint32_t));
    }
    if (write(serialIo,(void*)&data.opcode,1)<=0){
        return ERROR::NOTREADY;
    }
    if (data.dataLen>0){
        byte in[data.dataLen];
        data.serializeData(in);
        if (write(serialIo,in,data.dataLen)<=0){
            return ERROR::NOTREADY;
        }
    }
    return ERROR::NONE;
}

std::shared_ptr<pSensor> iRobot::readData()
{
    byte type;
    //std::cout << "Waiting for data" << std::endl;
    if (readStable(&type,1)!=ERROR::NONE){
        std::cout << std::strerror(errno) <<std::endl;
        std::cout << "Receive disconnected"<<std::endl;
        return std::shared_ptr<pSensor>();
    }

    //create the packet...
    auto pkt = pSensor::getSensorPacketManaged(type);
    if (pkt == 0){
        std::cout << "No implemented packet, out of sync..." << type<<std::endl;
        return std::shared_ptr<pSensor>();
    }

    int length = pkt->getLength();
    if (length == -1){
        //return the packet... it deals with itself...
        return pkt;
    }
    byte data[length];
    if (readStable(data,length)==ERROR::NONE){
        if (pkt->deserialise(type,data)!=ERROR::NONE){
            return std::shared_ptr<pSensor>();
        }
        return pkt;
    }
    return std::shared_ptr<pSensor>();
}

error iRobot::readStable(byte *data, int length)
{
    int total= 0;
    while(total!=length){
        int len = read(serialIo,&data[total],1);
        if (len <0){
            return ERROR::BADDATA;
        }
        else {
            total+=len;
        }
    }
    return ERROR::NONE;
}


error iRobot::disconnect()
{
    close(serialIo);
    return ERROR::NONE;
}
