#ifndef WIFI_SERVICES
#define wIFI_SERVICES



class WifiServices
{
public:
    WifiServices(/* args */);
    ~WifiServices();
    /* Connect to wifi */
    void connect(const char* ssid,const char *password);
    void disconnect();
    /* Create  access point*/
    void creatAP(const char* ssid,const char *password);
    void logNearWifi();
    
private:    
};





#endif // !WIFI_SERVICES