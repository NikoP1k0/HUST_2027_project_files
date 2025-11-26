#include "SD_card.h"
#include <string.h>
#include <time.h>
#include <nvs_flash.h>

const char *filePath;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){

  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){

  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void write2SDcard(const char* msg, int flag) {

  int8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  //uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  //Serial.printf("SD Card Size: %lluMB\n", cardSize);

  static char lastFilePath[30] = "/log_1.txt";  // Store last used file path
  char newFilePath[30];

  // Using flash memory to fetch last used lognumber
  
  if (flag) {
    // Retrieve the last log number from NVS
    int lastLogNumber = getLastLogNumber();
    int fileNumber = lastLogNumber + 1;

    // Generate a unique filename
    snprintf(newFilePath, sizeof(newFilePath), "/log_%d.txt", fileNumber);
    strcpy(lastFilePath, newFilePath); // Store last created file path

    // Update the last log number in NVS
    setLastLogNumber(fileNumber);

    writeFile(SD, newFilePath, msg);
  } else {
    // Append to last used file
    appendFile(SD, lastFilePath, msg);
  }

  // Iterating through files until one is found that has not been used
  /* if (flag) { 
    // Generate a unique filename
    int fileNumber = 1;
    do {
      snprintf(newFilePath, sizeof(newFilePath), "/log_%d.txt", fileNumber);
      fileNumber++;
      //Serial.printf("Checking file: %s\n", newFilePath);
    } while (SD.exists(newFilePath)); // Keep incrementing until we find a non-existing file
    
    strcpy(lastFilePath, newFilePath); // Store last created file path
    writeFile(SD, newFilePath, msg);
  } else {
    // Append to last used file
    appendFile(SD, lastFilePath, msg);
  } */

  //Serial.printf("Using file: %s\n", lastFilePath);
  //Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

bool initNVS() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    const esp_partition_t* nvs_partition = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    if (nvs_partition) {
      err = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
      if (err != ESP_OK) {
        Serial.println("Failed to erase NVS partition");
        return false;
      }
    }
    // Retry nvs_flash_init
    err = nvs_flash_init();
  }
  return err == ESP_OK;
}

int getLastLogNumber() {
  int32_t lastLogNumber = 0;
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.println("Error opening NVS");
    return 0;
  }
  err = nvs_get_i32(my_handle, NVS_KEY, &lastLogNumber);
  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
    Serial.println("Error reading last log number");
  }
  nvs_close(my_handle);
  return lastLogNumber;
}

void setLastLogNumber(int logNumber) {
  nvs_handle_t my_handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK) {
    Serial.println("Error opening NVS");
    return;
  }
  err = nvs_set_i32(my_handle, NVS_KEY, logNumber);
  if (err != ESP_OK) {
    Serial.println("Error writing last log number");
  }
  nvs_commit(my_handle);
  nvs_close(my_handle);
}
