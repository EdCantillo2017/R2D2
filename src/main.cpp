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
//    robot.reset();
    robot.modesafe(); // place iRobot inot safe mode

    char dir = '!';

// I am not sure what this section code does so let comment out for now
// I believe it uses the curse.h library
#if 0
    initscr();
    timeout(-1);
    raw();
    cbreak();
    noecho();
    refresh();
#endif


    std::cout << "Hello World! \n\r" << std::endl;
    robot.sensorStart();
   robot.registerCallback({SENSOR::bumpcleft }, [](std::shared_ptr<pSensor> data)
   {
                int32_t din;
                std::cout << "registerd call back \n\r" << std::endl;
                if (data->getData(din)!=ERROR::NONE)
                {
                    std::cout << "Data not valid" <<std::endl;
                }

//                if (data->getType() == SENSOR::bumpcleft)
                {
                    Mybumpcleft = din;
                    std::cout << "bump c left:" << din <<std::endl;
                    std::cout << "\n\r";

                }

//                if (data->getType() == SENSOR::bumpcright)
//                {
//                    Mybumpcright = din;
//                    std::cout << "bump c right:" << din <<std::endl;
//                    std::cout << "\n\r";
//                }

   });

    robot.sstream({SENSOR::bumpcleft});
    
    while (dir!='e')
    {
        std::cin >> dir;
        switch(dir)
        {
        case 'g':
            robot.registerCallback({SENSOR::bumpcleft, SENSOR::bumpcright}, [](std::shared_ptr<pSensor> data)
            {
                int32_t din;
                if (data->getData(din)!=ERROR::NONE)
                {
                    std::cout << "Data not valid" <<std::endl;
                }

                if (data->getType() == SENSOR::bumpcleft)
                {
                    Mybumpcleft = din;
                    std::cout << "bump c left:" << din <<std::endl;
                    std::cout << "\n\r";

                }

                if (data->getType() == SENSOR::bumpcright)
                {
                    Mybumpcright = din;
                    std::cout << "bump c right:" << din <<std::endl;
                    std::cout << "\n\r";
                }

            });

        break; // 'g'

        case '1':
            robot.spause();
            break;

        case '2':
            robot.sstream({SENSOR::bumpcleft,SENSOR::bumpcright});
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
    robot.reset();
}


