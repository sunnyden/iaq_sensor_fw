//
// Created by sunny on 10/13/22.
//

#ifndef IAQ_SENSOR_FW_SENSORDATA_H
#define IAQ_SENSOR_FW_SENSORDATA_H


class SensorData{
public:
    static void push_pt_value(float pm1d0,float pm25,float pm10, float tvoc, float hcho, float co2, float temperature, float humidity);
    static void pt_data_process();
    static float pm25;
    static float pm1d0;
    static float pm10;
    static float tvoc;
    static float co2;
    static float hcho;
    static float temperature;
    static float humidity;
    static int pt_sample_cnt;
    static bool isProcessed;
    static unsigned int bat_vol;

};


#endif //IAQ_SENSOR_FW_SENSORDATA_H
