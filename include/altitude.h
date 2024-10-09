#ifndef ALTITUDE_H
#define ALTITUDE_H

struct Altitude {
    float temperature;
    float pressure;
    float altitude;
};
// Function declarations for initializing altiude sensor
void initAltitude();
Altitude getAltitude();


#endif // ALTITUDE_H
