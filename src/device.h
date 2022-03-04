#ifndef DEVICE_H
#define DEVICE_H


#include "fsServices.h"

extern FsServices fsServices;

class Device
{

public:
    Device(/* args */) {}
    ~Device() {}
    /* Device initialization */
    void init();
        /* Creat AP server */
     void launchServer();
     void connectToWifi();
    /* Restart device */
    void restrat();
    /* Reset device to manufactory settings
     * delete all data saved in device memory
     * delete device from database
     * delete device from user owned devices
     */

    void reset();
    /* run device */
    void run();

private:
    /* Sign in with email and password*/
    void _signInWithEmailAndPassword();

};

#endif // !DEVICE_H