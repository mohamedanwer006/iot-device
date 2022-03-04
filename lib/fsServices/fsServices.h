#ifndef FS_SERVICES_H
#define FS_SERVICES_H
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>


class FsServices{
public:
    //* Init file system
    void init();
    //* Read file as string
    String readFile(const char *path);
    //* Write file as string
    bool writeFile(const char *path, const char *file); 
    //* Check if file exist using its path
    bool isFileExist(const char *path);
    //* Remove file using its path
    bool removeFile(const char *path);

private:
     bool _fsBegin();
    
};

#endif // !FS_SERVICES_H