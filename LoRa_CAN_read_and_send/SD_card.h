#ifndef SD_CARD
#define SD_CARD

#include <FS.h>
#include <SD.h>
#include <SPI.h>

// Define chip select for SD card
#define CS 14

// NVS namespace and key
#define NVS_NAMESPACE "storage"
#define NVS_KEY "last_log_number"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void testFileIO(fs::FS &fs, const char * path);
void write2SDcard(const char* msg, int flag);
bool initNVS();
int getLastLogNumber();
void setLastLogNumber(int logNumber);

#endif

