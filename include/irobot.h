#ifndef IROBOT_H
#define IROBOT_H

#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <memory>
#include <map>
#include <thread>
#include <functional>
#include "packets.h"
#include "error.h"
#include "psensor.h"

class iRobot
{
    friend class pMasterPkt;//for the weird packet in packet format
public:
    /**
     * @brief iRobot Create a instance of the robot
     */
    iRobot();

    /**
     * @brief start Start a new connection to a serial device
     * @param streamDevice The serial device to connect to.
     * @return See ERROR::
     */
    error start       (std::string streamDevice);

    /**
     * @brief startNetwork Start a new network-to-serial connection
     * @param host The host to connect to
     * @param port THe port to connect to
     * @return See ERROR::
     */
    error startNetwork(std::string host, uint16_t port);

    /**
     * @brief reset Reset the robot
     * @return See ERROR::
     */
    error reset       ();

    /**
     * @brief stop Stop all Robot motion
     * @return See ERROR::
     */
    error stop        ();

    /**
     * @brief baud Set the new serial Baud rate
     * @param dataRate The new serial rate see iCreate2 Documentation
     * @return See ERROR::
     */
    error baud        (byte dataRate);

    /**
     * @brief modesafe Switch into safe mode
     * @return See ERROR::
     */
    error modesafe    ();

    /**
     * @brief modefull Swtich into Full Mode
     * @return See ERROR::
     */
    error modefull    ();

    /**
     * @brief clean Start Cleaning
     * @return See ERROR::
     */
    error clean       ();

    /**
     * @brief max Start Cleaning until battery dies
     * @return See ERROR::
     */
    error max         ();

    /**
     * @brief spot Look for dirt?
     * @return See ERROR::
     */
    error spot        ();

    /**
     * @brief seekDock Find the dock
     * @return The dock we are looking for
     */
    error seekDock    ();

    /**
     * @brief power set the power schedule
     * @return See ERROR::
     */
    error power       ();

    /**
     * @brief schedule Set the device schedule
     * @param data the Schedule to set
     * @return See ERROR::
     */
    error schedule    (const pSchedule& data);

    /**
     * @brief settime Set the roomba's time
     * @param data The time to set
     * @return See ERROR::
     */
    error settime     (const pDate& data);

    /**
     * @brief drive Move the Roomba
     * @param velocity The velocity to move
     * @param radius The radius to turn
     * @return See ERROR::
     */
    error drive       (uint16_t velocity,uint16_t radius);

    /**
     * @brief dStraight Drive Straight
     * @param velocity Velocity to move
     * @return See ERROR::
     */
    error dStraight   (uint16_t velocity);

    /**
     * @brief dClockwise Turn Clockwise
     * @param velocity The Velocity to turn
     * @return See ERROR::
     */
    error dClockwise  (uint16_t velocity);

    /**
     * @brief dCClockwise Turn Counter Clockwise
     * @param velocity The velocity to move
     * @return See ERROR::
     */
    error dCClockwise (uint16_t velocity);

    /**
     * @brief driveDirect Drive, control wheels individually
     * @param rightVelocity the right wheel velocity
     * @param leftVelocity The left whell velocity
     * @return See ERROR::
     */
    error driveDirect (uint16_t rightVelocity, uint16_t leftVelocity);

    /**
     * @brief drivePWN Drive using Pulse width modulation
     * @param rightPWM The right wheel pwm
     * @param leftPWM The left wheel pwm
     * @return See ERROR::
     */
    error drivePWN    (uint16_t rightPWM, uint16_t leftPWM);

    /**Removed Motors**/
    //error motor       ();
    //error motorPWN    ();

    /**
     * @brief led Control the LEDs
     * @return See ERROR::
     */
    error led         ();

    /**
     * @brief ledSched Set the LED schedule
     * @return See ERROR::
     */
    error ledSched    ();

    /**
     * @brief ledDigit Set the LED digit
     * @return See ERROR::
     */
    error ledDigit    ();

    /**
     * @brief buttons Control the button state
     * @return See ERROR::
     */
    error buttons     ();

    /**
     * @brief digitLED Set the digit LED
     * @return See ERROR::
     */
    error digitLED    ();

    /**
     * @brief song Store a new song
     * @param data The song to store
     * @return See ERROR::
     */
    error song        (const pSong& data);

    /**
     * @brief play Play a stored song
     * @param songNo The song to play
     * @return See ERROR::
     */
    error play        (uint8_t songNo);

    /**
     * @brief sensors Request a sensor reading
     * @warning You must register a callback before calling
     * @return See ERROR::
     */
    error sensors     (uint8_t sensor);

    /**
     * @brief squerylist Query the list of sensors
     * @warning You must register a callback before calling
     * @param list The list of sensor packets
     * @return See ERROR::
     */
    error squerylist  (std::initializer_list<uint8_t> list);

    /**
     * @brief sstream Start a new Sensor stream
     * @warning You must register a callback before calling
     * @param list The list of sensor packets
     * @return See ERROR::
     */
    error sstream     (std::initializer_list<uint8_t> list);

    /**
     * @brief spause Pause the current sensor stream
     * @warning You must register a callback before calling
     * @return See ERROR::
     */
    error spause      ();

    /**
     * @brief sresume Resume a stream of data
     * @warning You must register a callback before calling
     * @return See ERROR::
     */
    error sresume     ();

    /**
     * @brief sensorStart Prepare for sensor data
     * @return See ERROR::
     */
    error sensorStart();

    /**
     * @brief registerCallback
     * @param sensor The sensor to attachto
     * @param ptr The funciton to call
     */
    void registerCallback(std::initializer_list<byte> sensors, std::function<void(std::shared_ptr<pSensor> callbacks)> function);



private:
    bool isNetwork = false;
    std::string connection;
    int serialIo;
    error writeData(const rData &data);
    std::shared_ptr<pSensor> readData();
    error readStable(byte* data,int length);
    error disconnect();

    std::map<byte,std::function <void(std::shared_ptr<pSensor>)>> callbacks;
    std::thread readThread;
};

#endif // IROBOT_H
