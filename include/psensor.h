#ifndef PSENSOR_H
#define PSENSOR_H
#include "error.h"
#include <memory>
#include <iostream>
#include <cstring>
namespace SENSOR {
    const byte masterpkt    = 19;
    const byte wheel_bump   =  7;
    const byte wall         =  8;
    const byte cliff_left   =  9;
    const byte cliff_fleft  = 10;
    const byte cliff_fright = 11;
    const byte cliff_right  = 12;
    const byte virtwall     = 13;
    const byte wheelovamps  = 14;
    const byte dirt         = 15;
    const byte unused       = 16;
    const byte ircharomni   = 17;
    const byte ircharleft   = 52;
    const byte ircharright  = 53;
    const byte buttons      = 18;
    /*const byte distance     = 19; - Disabled due to conflicting packet id's*/
    const byte angle        = 20;
    const byte charging     = 21;
    const byte voltage      = 22;
    const byte current      = 23;
    const byte temperature  = 24;
    const byte charge       = 25;
    const byte capacity     = 26;
    const byte wallsignal   = 27;
    const byte cliffleft    = 28;
    const byte clifffrleft  = 29;
    const byte clifffrright = 30;
    const byte cliffright   = 31;
    const byte chargesrc    = 34;
    const byte mode         = 35;
    const byte songno       = 36;
    const byte songply      = 37;
    const byte streamptcount= 38;
    const byte velocity     = 39;
    const byte radius       = 40;
    const byte rightvelo    = 41;
    const byte leftvelo     = 42;
    const byte leftencoder  = 43;
    const byte rightencoder = 44;
    const byte lightbumper  = 45;
    const byte bumpleft     = 46;
    const byte bumpfleft    = 47;
    const byte bumpcleft    = 48;
    const byte bumpcright   = 49;
    const byte bumpfright   = 50;
    const byte bumpright    = 51;
    const byte motorleftamp = 54;
    const byte motorrightamp= 55;
    const byte mainbrushamp = 56;
    const byte sidebrushamp = 57;
    const byte stasis       = 58;

}

class iRobot;
class pSensor
{
    friend class iRobot;
    int type;
    byte length;
    bool isSigned;
    byte* data;

public:
    pSensor(byte type,int length,bool isSigned=false){
        this->type = type;
        this->length = length;
        this->isSigned = isSigned;
        this->data = 0;
    }

    ~pSensor(){
        if (data != 0){
            delete[] data;
        }
    }

    int getLength(){
        return length;
    }

    byte getType(){
        return type;
    }



    /**
     * @brief getData
     * @return
     * @note Theres no 32 bit number in the protocol... so export a signed 32 bit
     */
    error getData(int32_t &dout){
        if (isSigned){
            switch (length){
            case 1:
                dout = *((int8_t*)data);
                break;
            case 2:
                dout = __builtin_bswap16(*((int16_t*)data));
                break;
            default:
                return ERROR::BADDATA;
            }
        }
        else {
            switch (length){
            case 1:
                dout = *((uint8_t*)data);
                break;
            case 2:
                dout = __builtin_bswap16(*((uint16_t*)data));
                break;
            default:
                return ERROR::BADDATA;
            }
        }
        return ERROR::NONE;
    }

    virtual error streamToMe(iRobot* robot){
        (void)robot;
        return ERROR::NOTIMPLEMENTED;
    }
public:
    static std::shared_ptr<pSensor>getSensorPacketManaged(int type){
        return std::shared_ptr<pSensor>(getSensor(type));
    }

private:
    static pSensor* getSensor(int type);

protected:
    virtual error deserialise(byte type,byte* data){
        if (type!= this->type){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }

        this->data = new byte[length];
        std::memcpy(this->data,data,length);
        return ERROR::NONE;
    }

};

class pSenseBump:public pSensor{

public:
    pSenseBump():pSensor(SENSOR::wheel_bump,1){

    }

public:
    bool isBumpRight;
    bool isBumpLeft;
    bool isWheelRight;
    bool isWheelLeft;

    // pSensor interface
protected:
    error deserialise(byte type, byte *data){
        if (type!= SENSOR::wheel_bump){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }
        isBumpRight  = (*data)&0x01;
        isBumpLeft   = (*data)&0x02;
        isWheelLeft  = (*data)&0x04;
        isWheelRight = (*data)&0x08;
        return ERROR::NONE;
    }
};


class pOvrCurrentBump:public pSensor{

public:
    pOvrCurrentBump():pSensor(SENSOR::wheelovamps,1){

    }

public:
    bool sideBrush;
    bool mainBrush;
    bool rightWheel;
    bool leftWheel;

    // pSensor interface
protected:
    error deserialise(byte type, byte *data){
        if (type!= SENSOR::wheelovamps){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }
        sideBrush   = (*data)&0x01;
        mainBrush   = (*data)&0x04;
        rightWheel  = (*data)&0x08;
        leftWheel   = (*data)&0x016;
        return ERROR::NONE;
    }
};

class pButton:public pSensor{

public:
    pButton():pSensor(SENSOR::buttons,1){

    }

public:
    bool clean;
    bool spot;
    bool dock;
    bool minute;
    bool hour;
    bool day;
    bool schedule;
    bool clock;

    // pSensor interface
protected:
    error deserialise(byte type, byte *data){
        if (type!= SENSOR::buttons){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }
        clean      = (*data)&0x01;
        spot       = (*data)&0x02;
        dock       = (*data)&0x04;
        minute     = (*data)&0x08;
        hour       = (*data)&0x016;
        day        = (*data)&0x032;
        schedule   = (*data)&0x064;
        clock      = (*data)&0x128;
        return ERROR::NONE;
    }
};

class pCharge:public pSensor{

public:
    enum{NONE=0,RECONDITION=1,FULL=2,TRICKLE=3,WAITINGFORCHRITMAS=4,FAULT=5};
    pCharge():pSensor(SENSOR::charging,1){

    }

};

class pChargeSrc:public pSensor{

public:
    pChargeSrc():pSensor(SENSOR::chargesrc,1){

    }

public:
    bool internal;
    bool base;

    // pSensor interface
protected:
    error deserialise(byte type, byte *data){
        if (type!= SENSOR::chargesrc){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }
        internal   = (*data)&0x01;
        base       = (*data)&0x04;
        return ERROR::NONE;
    }
};

class pMode:public pSensor{

public:
    enum{OFF=0,PASSIVE=1,SAFE=2,FULL=3};
    pMode():pSensor(SENSOR::mode,1){

    }
};

class pBumper:public pSensor{

public:
    pBumper():pSensor(SENSOR::lightbumper,1){

    }

public:
    bool left  ;
    bool fleft ;
    bool cleft ;
    bool cright;
    bool fright;
    bool right ;

    // pSensor interface
protected:
    error deserialise(byte type, byte *data){
        if (type!= SENSOR::lightbumper){
            std::cout << "OOPS... invalid data.. not a wheel drop packet"<<std::endl;
            return ERROR::BADDATA;
        }
        left       = (*data)&0x01;
        fleft      = (*data)&0x02;
        cleft      = (*data)&0x04;
        cright     = (*data)&0x08;
        fright     = (*data)&0x016;
        right      = (*data)&0x032;

        return ERROR::NONE;
    }
};

class iRobot;
class pMasterPkt: public pSensor{

public:
    pMasterPkt();
    error streamToMe(iRobot* robot);
};

#endif // PSENSOR_H
