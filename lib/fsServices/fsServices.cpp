#include "fsServices.h"
// init services
void FsServices ::init()
{
    if (!_fsBegin())
    {
        Serial.println("LittleFS mount failed");
        return;
    }
    Serial.println("LittleFS mount success ");
}

String FsServices ::readFile(const char *path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return "";
    }

    Serial.print("Read from file: ");
    String data;
    while (file.available())
    {
        data += char(file.read());
        // Serial.write(file.read());
    }
    Serial.println(data);
    file.close();
    return data;
}
bool FsServices ::writeFile(const char *path, const char *data)
{
    Serial.printf("Writing file: %s\n", path);

    File file = LittleFS.open(path, "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }
    if (file.print(data))
    {
        Serial.println("File written");
        delay(2000);
        file.close();
        return true;
    }
    else
    {
        Serial.println("Write failed");
    }
    return false;
}
bool FsServices ::isFileExist(const char *path)
{
    return LittleFS.exists(path);
}
bool FsServices ::removeFile(const char *path)
{
    if (!LittleFS.remove(path))
    {
        Serial.println("failed to remove file");
        return false;
    }
    return true;
}
bool FsServices ::_fsBegin()
{
    return LittleFS.begin();
}
