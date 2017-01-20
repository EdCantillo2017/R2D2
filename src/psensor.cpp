#include "psensor.h"
#include "irobot.h"
pSensor *pSensor::getSensor(int type)
{
    switch (type) {
    case SENSOR::masterpkt:
        return new pMasterPkt();
    case SENSOR::wheel_bump   :
        return new pSenseBump();
    case SENSOR::wall         :
        return new pSensor(type,1);
    case SENSOR::cliff_left   :
        return new pSensor(type,1);
    case SENSOR::cliff_fleft  :
        return new pSensor(type,1);
    case SENSOR::cliff_fright :
        return new pSensor(type,1);
    case SENSOR::cliff_right  :
        return new pSensor(type,1);
    case SENSOR::virtwall     :
        return new pSensor(type,1);
    case SENSOR::wheelovamps  :
        return new pOvrCurrentBump();
    case SENSOR::dirt         :
        return new pSensor(type,1);
    case SENSOR::unused       :
        return new pSensor(type,1);
    case SENSOR::ircharomni   :
        return new pSensor(type,1);
    case SENSOR::ircharleft   :
        return new pSensor(type,1);
    case SENSOR::ircharright  :
        return new pSensor(type,1);
    case SENSOR::buttons      :
        return new pButton();
//    case SENSOR::distance     :
//        return new pSensor(type,2,true);
    case SENSOR::angle        :
        return new pSensor(type,2,true);
    case SENSOR::charging     :
        return new pCharge();
    case SENSOR::voltage      :
        return new pSensor(type,2);
    case SENSOR::current      :
        return new pSensor(type,2,true);
    case SENSOR::temperature  :
        return new pSensor(type,1);
    case SENSOR::charge       :
        return new pSensor(type,2);
    case SENSOR::capacity     :
        return new pSensor(type,2);
    case SENSOR::wallsignal   :
        return new pSensor(type,2);
    case SENSOR::cliffleft    :
        return new pSensor(type,2);
    case SENSOR::clifffrleft  :
        return new pSensor(type,2);
    case SENSOR::clifffrright :
        return new pSensor(type,2);
    case SENSOR::cliffright   :
        return new pSensor(type,2);
    case SENSOR::chargesrc    :
        return new pChargeSrc();
    case SENSOR::mode         :
        return new pMode();
    case SENSOR::songno       :
        return new pSensor(type,1);
    case SENSOR::songply      :
        return new pSensor(type,1);
    case SENSOR::streamptcount:
        return new pSensor(type,1);
    case SENSOR::velocity     :
        return new pSensor(type,2,true);
    case SENSOR::radius       :
        return new pSensor(type,2,true);
    case SENSOR::rightvelo    :
        return new pSensor(type,2,true);
    case SENSOR::leftvelo     :
        return new pSensor(type,2,true);
    case SENSOR::leftencoder  :
        return new pSensor(type,2);
    case SENSOR::rightencoder :
        return new pSensor(type,2);
    case SENSOR::lightbumper  :
        return new pBumper();
    case SENSOR::bumpleft     :
        return new pSensor(type,2);
    case SENSOR::bumpfleft    :
        return new pSensor(type,2);
    case SENSOR::bumpcleft    :
        return new pSensor(type,2);
    case SENSOR::bumpcright   :
        return new pSensor(type,2);
    case SENSOR::bumpfright   :
        return new pSensor(type,2);
    case SENSOR::bumpright    :
        return new pSensor(type,2);
    case SENSOR::motorleftamp :
        return new pSensor(type,2,true);
    case SENSOR::motorrightamp:
        return new pSensor(type,2,true);
    case SENSOR::mainbrushamp :
        return new pSensor(type,2,true);
    case SENSOR::sidebrushamp :
        return new pSensor(type,2,true);
    case SENSOR::stasis       :
        return new pSensor(type,1);
    default:
        return 0;
    }
}

pMasterPkt::pMasterPkt():pSensor(SENSOR::masterpkt,-1,true){

}

error pMasterPkt::streamToMe(iRobot *robot)
{
    byte total;
    if (robot->readStable(&total,1)!=ERROR::NONE){
        return ERROR::BADDATA;
    }

    while (total>0) {

        auto pktIn = robot->readData();
        if (!pktIn){
            std::cout << "Error in stream..."<<std::endl;
            return ERROR::BADDATA;
        }
        total -= pktIn->getLength();
        total -=1;
        auto result = robot->callbacks.find(pktIn->getType());
        if (result==robot->callbacks.end()){
            std::cout<<"No callback registered for packet type" << pktIn->getType()<<std::endl;
        }
        else {
            result->second(pktIn);
        }

    }
    byte checksum;
    if (robot->readStable(&checksum,1)!=ERROR::NONE){
        return ERROR::BADDATA;
    }
    return ERROR::NONE;
}
