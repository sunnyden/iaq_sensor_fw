//
// Created by sunny on 10/13/22.
//

#ifndef IAQ_SENSOR_FW_APICOMM_H
#define IAQ_SENSOR_FW_APICOMM_H
#include "SensorData.h"
#include "json.h"

class ApiComm{
public:
    static void upload();
    static bool hasPosted;
};


#endif //IAQ_SENSOR_FW_APICOMM_H
