#include <iostream>
#include <irobot.h>
#include <curses.h>
#include <psensor.h>
#include <unistd.h>
using namespace std;


void initSensorsForNav(iRobot &robot);

int Mybumpfright;
int Mybumpfleft;
int Mybumpcright;
int Mybumpcleft;
int Mybumpright;
int Mybumpleft;
int MyRightEncoder;
int MyLeftEncoder;
int MyAngle;
int i = 0;
int disTraveled;
int StartTravel;
int StartTurnLeft;
int StartTurnRight;
int AmountToTurn;


#define DriveSpeed      150
#define TurnSpeed       150
#define LowBumpThres    500
#define HighBumpThres   2000
#define NoWall          50
#define ExitSpeed       75

// defines for calculating dist and angle
#define Pi              3.1416
#define WheelBase       235
#define WheelDia        72
#define rad2Deg(x)      (x*180)/Pi


typedef enum
{
    R_INIT,
    R_DRIVE,
    R_TURN,
    R_FIND_MIDDLE,
    R_FIND_WALL,
    R_FOLLOW_WALL_RIGHT,
    R_FOLLOW_WALL_LEFT,
    R_EXIT_ROOM,
    R_ANGLE,
    R_DISTANCE,
    R_STOP
} _robotNavStates;


_robotNavStates robotNavState;


int main()
{

    cout << "Hello World!" << endl;

    iRobot  robot;
 
 // this section of code is not required for this project
 // the network is not required because we are running from the Pi   
#if 0
    std::string ip;
    cout << "Ip:";
    cin  >> ip;

    if (robot.startNetwork(ip,2468)!=ERROR::NONE){
        std::cout << "NO network path found";
        return -1;
    }
#endif

    robot.start("/dev/ttyUSB0"); // open the serial com port on the RPi      
    robot.modesafe(); // place iRobot inot safe mode 

    int dir = '!';

// I am not sure what this section code does so let comment out for now     
// I believe it uses the curse.h library 
#if 1     
    initscr();
    timeout(-1);
    raw();
    cbreak();
    noecho();
#endif     
    
    while (dir!='e')
    {
        dir = getch();
        switch(dir)
        {
        case 'g':

            while (1)
            {
                switch(robotNavState)
                {
                case R_INIT:
                    initSensorsForNav(robot);
                    robotNavState = R_DRIVE;
                    robot.play(1);  // feed back
                    break;

                case R_DRIVE:
                    robot.dStraight(DriveSpeed); // medium speed
                    disTraveled = 0;
                    robotNavState = R_FIND_WALL;
                    break;

                case R_TURN:
                    MyAngle = rad2Deg( (((MyRightEncoder*Pi*72)/508.8)-((MyLeftEncoder*Pi*72)/508.8))/235 );// in radians
                    if (MyAngle >= AmountToTurn)
                    {
                        robot.dStraight(DriveSpeed);
                        robotNavState = R_FOLLOW_WALL_RIGHT;
                    }

                    break;

                case R_FIND_WALL:
                    if ((Mybumpfright > HighBumpThres ) || (Mybumpcright > HighBumpThres ) || (Mybumpright > HighBumpThres ) ||
                        (Mybumpfleft > HighBumpThres )  || (Mybumpcleft > HighBumpThres)   || (Mybumpleft > HighBumpThres)    )
                    {
                        robot.dCClockwise(TurnSpeed); // turn left
                        StartTurnLeft = MyLeftEncoder;
                        StartTurnRight = MyRightEncoder;
                        AmountToTurn = 90;
                        robotNavState = R_TURN;
                    }
                    break;

                case R_FIND_MIDDLE:

                    if ((MyRightEncoder - StartTravel) >= 2742 ) // about 4 feet
                    {
                        robot.dStraight(0);
                        robot.dClockwise(TurnSpeed);
                        usleep(1025*1000);
                        robotNavState = R_FIND_WALL;
                        robot.dStraight(DriveSpeed);
                    }
                    else
                    {
                        if ((Mybumpfright > HighBumpThres ) || (Mybumpcright > HighBumpThres ) || (Mybumpright > HighBumpThres ) ||
                            (Mybumpfleft > HighBumpThres )  || (Mybumpcleft > HighBumpThres)   || (Mybumpleft > HighBumpThres)    )
                        {
                            robot.dClockwise(TurnSpeed); // turn left
                            usleep(2750*1000); // turn 180
                            StartTravel = MyRightEncoder;
                            robotNavState = R_FIND_MIDDLE;
                            robot.dStraight(DriveSpeed);
                        }
                    }

                    break;


                case R_FOLLOW_WALL_RIGHT:
                    if ((Mybumpcright > HighBumpThres) && (Mybumpcleft > HighBumpThres))
                    {  // turn 90 degrees this is a corner
                        robot.dCClockwise(TurnSpeed);// turn left
                        sleep(1.25); // 1.5 sec
                    }
                    else if ((Mybumpright > LowBumpThres) && (Mybumpright < HighBumpThres))
                    {
                        robot.dStraight(DriveSpeed);

                    }
                    else if ((Mybumpright <= NoWall))
                    {
                        robot.dStraight(0); // stop
                        sleep(4);
                        robotNavState = R_EXIT_ROOM;
                    }
                    break;

                case R_FOLLOW_WALL_LEFT:
                    if ((Mybumpcright > HighBumpThres) && (Mybumpcleft > HighBumpThres))
                    {  // turn 90 degrees this is a corner
                        robot.dClockwise(TurnSpeed);// turn left
                        sleep(1.25); // 500msec
                    }
                    else if ((Mybumpleft > LowBumpThres) && (Mybumpleft < HighBumpThres))
                    {
                        robot.dStraight(DriveSpeed);

                    }
                    else if ((Mybumpleft <= NoWall))
                    {
                        robot.dStraight(0); // stop
                        sleep(4);
                        robotNavState = R_EXIT_ROOM;
                    }

                    break;

                case R_EXIT_ROOM:
                    robot.dStraight(DriveSpeed);
                    sleep(4); // move up just a bit to clear the wall
                    robot.dClockwise(DriveSpeed);
                    sleep(1.25);
                    robot.dStraight(DriveSpeed);
                    sleep(4);
                    robotNavState = R_STOP;
                    robot.play(1);
                    break;

                case R_STOP:
                    robot.dStraight(0); // stop
                    break;

                case R_ANGLE:
                    // angle = ((rightEncoder*Pi*72/508.8)-(leftEncoder*Pi*72/508.0))/235



                case R_DISTANCE:
                    // dist = encoder_reading * (Pi*72/508.8)


                default:
                    break;

                }
            }

        break; // 'g'

        case '1':
            robot.spause();
            break;

        case '2':
            robot.sstream({SENSOR::bumpfleft,SENSOR::bumpfright});
            break;
#if 1
        case '3':
            robot.squerylist({SENSOR::bumpfleft,SENSOR::bumpfright});
            break;
#endif
        case 'w':
            cout << "straight" << endl;
            robot.dStraight(250);
            break;
        case 'a':
            //Place turning code here
            robot.dCClockwise(250);
            break;
        case 's':
            robot.dStraight(-250);
            break;
        case 'd':
            //Place turning code here
            robot.dClockwise(250);
            break;
        case '/':
            robot.seekDock();
            break;
        case 'x':
            robot.modesafe();
            break;
        case 'm':
            robot.modefull();
            break;
        case 'z':
            robot.start("/dev/ttyUSB0");
            break;
        case 'n':
            // not using the network for this project
            //robot.startNetwork(ip,2468);
            break;
        case 'p':
            robot.dStraight(0);
            break;
        case 't':
            robot.stop();
            break;
        case 'v':
            endwin();
            break;
        case 'b':
        {
            using namespace psong;
            uint8_t* notes = new uint8_t[5]
            {60,69,72,69,72};
             /*b,b,c,d,d,c,b,a,g,g,a,b,a,g,g,
             a,a,b,g,a,b,c,b,g,a,b,c,d,a,g,a,d,
             b,b,c,d,d,c,b,a,g,g,a,b,a,g,g};*/
            uint8_t* len   = new uint8_t[5]
            {8,8,16,8,16};
             /*1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
             1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
             1,1,1,1,1,1,1,1,1,1,1,1,1,1,2};*/
            uint8_t* scaleIn = new uint8_t[15]
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

            for (int i=0;i!=5;i++){
                len[i] =len[i];
            }

            pSong song(1);
            song.setSong(notes,len,scaleIn,5);
            robot.song(song);
            break;
        }
        case '.':
            robot.play(1);
        }

    }

}

// start streaming a bunch of sensors to navigate the robot out of the room
void initSensorsForNav(iRobot &_robot)
{
    _robot.registerCallback({SENSOR::rightencoder}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        MyRightEncoder = din;
        std::cout << "right encoder:" << din <<std::endl;
        std::cout << "\n\r";
    });

    _robot.registerCallback({SENSOR::bumpfleft}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpfleft = din;
        std::cout << "bump front left:" << din <<std::endl;
        std::cout << "\n\r";
    });

    _robot.registerCallback({SENSOR::bumpfright}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpfright = din;
        std::cout << "bump front right:" << din <<std::endl;
        std::cout << "\n\r";
    });

    _robot.registerCallback({SENSOR::bumpcleft}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpcleft = din;
        std::cout << "bump center left:" << din <<std::endl;
        std::cout << "\n\r";
    });
    _robot.registerCallback({SENSOR::bumpcright}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpcright = din;
        std::cout << "bump center right:" << din <<std::endl;
        std::cout << "\n\r";
    });

    _robot.registerCallback({SENSOR::bumpleft}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpleft = din;
        std::cout << "bump left:" << din <<std::endl;
        std::cout << "\n\r";
    });

    _robot.registerCallback({SENSOR::bumpright}, [](std::shared_ptr<pSensor> data)
    {
        int32_t din;
        if (data->getData(din)!=ERROR::NONE)
        {
            std::cout << "Data not valid" <<std::endl;
        }
        Mybumpright = din;
        std::cout << "bump right:" << din <<std::endl;
        std::cout << "\n\r";
    });

    // needed to start the call back function. starts a read tread
    _robot.sensorStart();
    // start streaming the registered sensors
    _robot.sstream({SENSOR::rightencoder, SENSOR::bumpfleft,SENSOR::bumpfright,SENSOR::bumpcleft, SENSOR::bumpcright, SENSOR::bumpleft, SENSOR::bumpright});


    // setup to play a song
    using namespace psong;
    uint8_t* notes = new uint8_t[5]
    {60,69,72,69,72};
    uint8_t* len   = new uint8_t[5]
    {8,8,16,8,16};
    uint8_t* scaleIn = new uint8_t[15]
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    for (int i=0;i!=5;i++){
        len[i] =len[i];
    }
    pSong song(1);
    song.setSong(notes,len,scaleIn,5);
    _robot.song(song);
}





