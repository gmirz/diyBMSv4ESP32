#ifndef DIYBMSWebServer_H_
#define DIYBMSWebServer_H_

#include <stdio.h>

#include <esp_http_server.h>

#include "settings.h"
#include "FS.h"
#include "LittleFS.h"
#include "SD.h"
#include <SPIFFS.h>
#include "time.h"
#include "defines.h"
#include "Rules.h"
#include "settings.h"
#include "ArduinoJson.h"
#include "PacketRequestGenerator.h"
#include "PacketReceiveProcessor.h"

#include "EmbeddedFiles_AutoGenerated.h"
//#include "EmbeddedFiles_Integrity.h"
#include "HAL_ESP32.h"

int printBoolean(char *buffer, size_t bufferLen,const char *fieldName, boolean value, boolean addComma);
int printBoolean(char *buffer, size_t bufferLen, const char *fieldName, boolean value);

void generateUUID();

void StartServer();
void clearModuleValues(uint8_t module);
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);

void saveConfiguration();
void setNoStoreCacheControl(httpd_req_t *req);

esp_err_t SendSuccess(httpd_req_t *req);
esp_err_t SendFailure(httpd_req_t *req);
static esp_err_t ota_post_handler(httpd_req_t *req);


extern diybms_eeprom_settings mysettings;
extern bool _sd_card_installed;
extern TaskHandle_t avrprog_task_handle;
extern avrprogramsettings _avrsettings;
extern RelayState previousRelayState[RELAY_TOTAL];
extern currentmonitoring_struct currentMonitor;
extern void suspendTasksDuringFirmwareUpdate();
extern void resumeTasksAfterFirmwareUpdateFailure();

#endif