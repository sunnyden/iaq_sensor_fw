//
// Created by sunny on 10/13/22.
//

#include "SensorData.h"
float SensorData::pm1d0=0;
float SensorData::pm25=0;
float SensorData::pm10=0;
float SensorData::tvoc=0;
float SensorData::hcho=0;
float SensorData::co2=0;
float SensorData::temperature=0;
float SensorData::humidity=0;
int SensorData::pt_sample_cnt=0;
bool SensorData::isProcessed= false;
unsigned int SensorData::bat_vol= 0;
void
SensorData::push_pt_value(float pm1d0, float pm25, float pm10, float tvoc, float hcho, float co2, float temperature,
                          float humidity) {
    SensorData::pm1d0+=pm1d0;
    SensorData::pm25+=pm25;
    SensorData::pm10+=pm10;
    SensorData::tvoc+=tvoc;
    SensorData::hcho+=hcho;
    SensorData::co2+=co2;
    SensorData::temperature+=temperature;
    SensorData::humidity+=humidity;
    pt_sample_cnt++;

}

void SensorData::pt_data_process() {
    pm1d0/=(float)pt_sample_cnt;
    pm25/=(float)pt_sample_cnt;
    pm10/=(float)pt_sample_cnt;
    tvoc/=(float)pt_sample_cnt;
    hcho/=(float)pt_sample_cnt;
    co2/=(float)pt_sample_cnt;
    temperature/=(float)pt_sample_cnt;
    humidity/=(float)pt_sample_cnt;
    isProcessed= true;
}


