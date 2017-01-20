#ifndef PACKETS
#define PACKETS

#include "error.h"
#include <stdint.h>
#include <vector>
#include <cstring>


namespace OPCODE {
    const byte start        = 128;
    const byte reset        = 7;
    const byte stop         = 173;
    const byte baud         = 129;
    const byte safe         = 131;
    const byte full         = 132;
    const byte clean        = 135;
    const byte max          = 136;
    const byte spot         = 134;
    const byte seekDock     = 143;
    const byte power        = 133;
    const byte schedule     = 167;
    const byte settime      = 168;
    const byte drive        = 137;
    const byte driveDirect  = 145;
    const byte drivePWM     = 146;
    const byte motor        = 138;
    const byte motorPWM     = 144;
    const byte led          = 139;
    const byte ledSched     = 162;
    const byte ledDigit     = 163;
    const byte buttons      = 165;
    const byte digitLED     = 164;
    const byte song         = 140;
    const byte play         = 141;
    const byte sensors      = 142;
    const byte squerylist   = 149;
    const byte sstream      = 148;
    const byte spause       = 150;

}

/**
 * @brief The rData class Base class for a robot data
 */
class rData{

    friend class iRobot;
public:
    /**
     * @brief rData
     * @param opcode The OPCODE of the data se OPCODE::
     * @param dataLen The length of the data to send
     */
    rData(int opcode,int dataLen = 0) {
        this->opcode = opcode;
        this->dataLen = dataLen;
    }


protected:
    byte opcode;
    int dataLen;

    virtual error serializeData(byte* data) const{
        (void)data;
        return ERROR::NOTIMPLEMENTED;
    }
};

class pbaud:public rData{
public:
    /**
     * @brief pbaud Baud rate packet
     * @param rate The rate to set
     */
    pbaud(byte rate):rData(OPCODE::baud,1){
        this->rate = rate;
    }
private:
    byte rate;
    error serializeData(byte *data){
        data[0] = rate;
        return ERROR::NONE;
    }
};

class pSchedule: public rData{

public:
    enum day {SUNDAY = 0,MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY};

    /**
     * @brief pSchedule A schedule packet
     */
    pSchedule():rData(OPCODE::schedule,15){
        days  =0;
        for (int i = 0;i!=7;i++){
            hours[i] = 0;
            minutes[i] =0;
        }
    }

    /**
     * @brief addDay Add a day to the schedule
     * @param dayName The day
     * @param hour The hour
     * @param minute The minute
     */
    void addDay(day dayName,byte hour,byte minute){
        days |= (1<<(dayName-1));
        hours[dayName] = hour;
        minutes[dayName]= minute;
    }

private:
    byte days;
    byte hours  [7];
    byte minutes[7];


    error serializeData(byte *data){
        data[0] = days;
        int cursor = 1;
        for (int i = 0;i!=7;i++){
            data[cursor++] = hours[i];
            data[cursor++] = minutes[i];
        }
        return ERROR::NONE;
    }
};

class pDate:public rData{
    enum day {SUNDAY = 0,MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY};

public:
    /**
     * @brief pDate
     * @param dayName The day
     * @param hour The hour
     * @param minute The minute
     */
    pDate(day dayName,byte hour,byte minute):rData(OPCODE::settime){
        this->dayName = dayName;
        this->hour = hour;
        this->min = minute;
    }

private:
    byte dayName;
    byte hour;
    byte min;

    error serializeData(byte *data){
        data[0] = dayName;
        data[1] = hour;
        data[2] = min;
        return ERROR::NONE;
    }
};

class pDrive:public rData{

public:
    enum drive{STRAIGHT=0x8000,CW=0xFFFF,CCW=0x0001};

    /**
     * @brief pDrive A drive packet
     * @param rightOrVelocity
     * @param leftorRadius
     * @param type
     */
    pDrive(int16_t rightOrVelocity,int16_t leftorRadius,int type): rData(type,4){
        this->velocity = rightOrVelocity;
        this->radius = leftorRadius;
    }



    error serializeData(byte *data) const{
        uint16_t ovel = __builtin_bswap16(velocity);
        uint16_t orad = __builtin_bswap16(radius);
        *((uint16_t*)&data[0]) = ovel;
        *((uint16_t*)&data[2]) = orad;
        return ERROR::NONE;
    }
private:
    int16_t velocity;
    int16_t radius;
};

namespace psong{
    enum {g=0, G, a, A, b, c, C, d, D, e, f, F};
}

class pSong:public rData{

public:
    /**
     * @brief pSong
     * @param number The song No
     */
    pSong(int number):rData(OPCODE::song){
        this->songNo = number;
        this->notes = 0;
        this->length =0;
        this->duration =0;
    }
    ~pSong(){
        delete[] notes;
        delete[] duration;
    }
    pSong(const pSong& that) = delete;

    /**
     * @brief setSong
     * @param song The song to set
     * @param duration The note duration
     * @param scale The scale to use
     * @param length The length
     * @return
     */
    error setSong(uint8_t* song,uint8_t* duration,uint8_t* scale,int length){
        this->notes = song;
        this->duration = duration;
        this->length = length;
        this->dataLen = length*2+2;

        for (int i=0;i!=length;i++){
           notes[i] = notes[i]+(scale==0?0:scale[i]*12)+31;
        }
        if (scale)
            delete[] scale;

        return ERROR::NONE;
    }
    error serializeData(byte *data) const{
        data[0] = songNo;
        data[1] = length;

        int cursor = 2;
        for (int i=0;i!=length;i++){
            data[cursor++] = notes[i];
            data[cursor++] = duration[i];
        }
        return ERROR::NONE;
    }

private:
    byte songNo;
    uint8_t* notes;
    uint8_t* duration;
    uint8_t length;
};

class pPlay:public rData{
public:
    /**
     * @brief pPlay Plays a song in memory
     * @param number The memory bank to read
     */
    pPlay(uint8_t number):rData(OPCODE::play,1){
        this->number = number;
    }

    error serializeData(byte *data) const{
        data[0] = number;
        return ERROR::NONE;
    }

private:
    uint8_t number;
};

class pSensorRequest:public rData{
    uint8_t sensor;
public:
    pSensorRequest(int sensor):rData(OPCODE::sensors,1){
        this->sensor = sensor;
    }

    // rData interface
protected:
    error serializeData(byte *data) const{
        data[0] = sensor;
        return ERROR::NONE;
    }
};

class pQuery:public rData{
    std::vector<byte> list;

public:
    pQuery(std::initializer_list<uint8_t> list):rData(OPCODE::squerylist){
        rData::dataLen = list.size()+1;

        for (auto ptr = list.begin();ptr!=list.end();ptr++){
            this->list.push_back(*ptr);
        }
    }

protected:
    error serializeData(byte *data) const{
        data[0] = list.size();
        for (uint32_t i=0;i!=list.size();i++){
            data[i+1] = list[i];
        }
        return ERROR::NONE;
    }

};


class pStream:public rData{
    std::vector<byte> list;

public:
    pStream(std::initializer_list<uint8_t> list):rData(OPCODE::sstream){
        rData::dataLen = list.size()+1;

        for (auto ptr = list.begin();ptr!=list.end();ptr++){
            this->list.push_back(*ptr);
        }
    }

protected:
    error serializeData(byte *data) const{
        data[0] = list.size();
        for (uint32_t i=0;i!=list.size();i++){
            data[i+1] = list[i];
        }
        return ERROR::NONE;
    }

};

class pStrCtl:public rData{
    bool isPause;

public:
    pStrCtl(bool isPause):rData(OPCODE::spause,1){
        this->isPause = isPause;
    }

protected:
    error serializeData(byte* data){
        data[0] = isPause?0:1;
        return ERROR::NONE;
    }

};



#endif // PACKETS
