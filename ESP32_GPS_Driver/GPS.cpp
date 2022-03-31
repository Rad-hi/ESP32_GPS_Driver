/* 
 * -----------------------------------------------------------------------------
 * GPS Driver (wrapper for TinyGPS++) made for the ESP32 dev board.
 * -----------------------------------------------------------------------------
 * Author: Radhi SGHAIER: https://github.com/Rad-hi
 * -----------------------------------------------------------------------------
 * Date: 30-03-2022 (30th of March, 2022)
 * -----------------------------------------------------------------------------
 * License: Do whatever you want with the code ...
 *          If this was ever useful to you, and we happened to meet on 
 *          the street, I'll appreciate a cup of dark coffee, no sugar please.
 * -----------------------------------------------------------------------------
 */

#include "GPS.h"

/* Local data containers */
static LOCATION_t local_location;
static DATETIME_t local_datetime;
static float local_speed;
static uint8_t gps_err;

// Local reference to the GPS handler
GPS_t * local_gps_t;

/* Software timer params */
static SemaphoreHandle_t gps_data_mutex;
static TimerHandle_t timer_h = NULL;  // Timer handle
static TaskHandle_t read_gps_task_handle = NULL;

/* Flush the serial buffer */
static void serial_clean_buffer_gps(){
//  gps_t->serial_gps_handle->flush();
  while(local_gps_t->serial_gps_handle->available()) local_gps_t->serial_gps_handle->read();
}

/* Async reading functions */

// This function notifies the reading task when it's time to read
static void gps_timer_callback(TimerHandle_t timer_h){
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  vTaskNotifyGiveFromISR(read_gps_task_handle, &xHigherPriorityTaskWoken);
  
  if(xHigherPriorityTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

// Task that performs the sequencial reads of the GPS 
static void gps_read_task(void *params){

  const TickType_t max_blocking_time = pdMS_TO_TICKS(500); // ms
  
  while(1){
    
    ulTaskNotifyTake(pdTRUE, max_blocking_time);
    xTimerStart(timer_h, portMAX_DELAY); // Rewind the timer
    
    // To ensure that the readings' quality is maintained, call this frequently
    local_gps_t->gps->encode(local_gps_t->serial_gps_handle->read());
  
    // No GPS data available -> wiring/serial problem 
    if (local_gps_t->gps->charsProcessed() < 10){
      gps_err = ERR_WIRING_SERIAL;
      continue;
    }
  
    // There's one or more sentences that failed the checksum -> corrupted data
    if (local_gps_t->gps->failedChecksum() >= 1){
      gps_err = ERR_GPS_DATA_INVALID;
      serial_clean_buffer_gps();
      continue;
    }
  
    // No sentences have fix -> Go outdoor, check the antenna, or simply wait 
    if (local_gps_t->gps->sentencesWithFix() < 1){
      gps_err = ERR_NO_GPS_FIX;
      continue;
    }
  
    // All good --> Decode new data 
    if(local_gps_t->gps->location.isUpdated()){
      xSemaphoreTake(gps_data_mutex, portMAX_DELAY);
        local_location.lat = local_gps_t->gps->location.lat();
        local_location.lon = local_gps_t->gps->location.lng();
      xSemaphoreGive(gps_data_mutex);
    }
  
    if(local_gps_t->gps->date.isUpdated()){
      xSemaphoreTake(gps_data_mutex, portMAX_DELAY);
        local_datetime.dd = local_gps_t->gps->date.day();
        local_datetime.mm = local_gps_t->gps->date.month();
        local_datetime.yy = local_gps_t->gps->date.year();
      xSemaphoreGive(gps_data_mutex);
    }
  
    if(local_gps_t->gps->time.isUpdated()){
      xSemaphoreTake(gps_data_mutex, portMAX_DELAY);
        local_datetime.sc = local_gps_t->gps->time.second();
        local_datetime.mn = local_gps_t->gps->time.minute();
        local_datetime.hr = local_gps_t->gps->time.hour();
      xSemaphoreGive(gps_data_mutex);
    }
  
    if(local_gps_t->gps->speed.isUpdated()){
      xSemaphoreTake(gps_data_mutex, portMAX_DELAY);
        local_speed = local_gps_t->gps->speed.mps();
      xSemaphoreGive(gps_data_mutex);
    }
    gps_err = ERR_GPS_ALL_GOOD;
    
    xTimerStart(timer_h, portMAX_DELAY); // Rewind the timer
  }
}

/* Send commands to the GPS module */
static void serial_send_UBX_gps(GPS_t * gps_t, uint8_t * msg, uint8_t len){
  for(int i = 0; i < len; i++) gps_t->serial_gps_handle->write(msg[i]);
}

/* Setup */
void serial_init_gps(GPS_t * gps_t, 
                     TinyGPSPlus * gps_, 
                     #ifdef USE_SOFTWARE_SERIAL
                       SoftwareSerial * serial_port
                     #else
                       HardwareSerial * serial_port
                     #endif // USE_SOFTWARE_SERIAL
                     ){

  gps_t->serial_gps_handle = serial_port;
  gps_t->gps = gps_;

  #ifdef USE_SOFTWARE_SERIAL
    gps_t->serial_gps_handle->begin(GPS_BAUD_RATE, SWSERIAL_8N1, GPS_SOFT_SER_RX, GPS_SOFT_SER_TX, INVERT_LOGIC, BUF_SIZE);
  #else
    gps_t->serial_gps_handle->begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_SER_RX, GPS_SER_TX);
  #endif // USE_SOFTWARE_SERIAL

  // Get a local ref for the GPS object
  local_gps_t = gps_t;
  
  gps_data_mutex = xSemaphoreCreateMutex();
  // Create the software timer that'll trigger the read of the GPS data
  timer_h = xTimerCreate("read_gps_timer", GPS_READ_FREQUENCY/portTICK_PERIOD_MS, pdTRUE, (void*)0, gps_timer_callback);
  xTaskCreatePinnedToCore(gps_read_task, "read_GPS", 2048, NULL, 1, &read_gps_task_handle, 1);
}

/* This function gives back the most recent GPS data */
uint8_t serial_read_gps(GPS_t * gps_t, LOCATION_t * loc, DATETIME_t * datetime, float * speed_){

  xSemaphoreTake(gps_data_mutex, portMAX_DELAY);
  // Copy the most recent available data to the provided references
  if(loc != NULL) memcpy((void*)loc, (void*)&local_location, sizeof local_location);
  if(datetime != NULL) memcpy((void*)datetime, (void*)&local_datetime, sizeof local_datetime);
  if(speed_ != NULL) memcpy((void*)speed_, (void*)&local_speed, sizeof local_speed);
  xSemaphoreGive(gps_data_mutex);
  
  return gps_err;
}

// INPUT: locations (of type LOCATION_t), OUTPUT: distance between them in meters
void serial_distance_from_to_gps(LOCATION_t * from_, LOCATION_t * to_, float * distance){
  *distance = TinyGPSPlus::distanceBetween(from_->lat, from_->lon, to_->lat, to_->lon);
}

/* Wake up/Put to sleep functions */

void serial_put_to_sleep_gps(GPS_t * gps_t){
  /* Turn OFF RF section, Only use this if Sleep/wake cycles are longer than 30s */
  uint8_t gps_off[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x08, 0x00, 0x16, 0x74};
  serial_send_UBX_gps(gps_t, gps_off, sizeof(gps_off)/sizeof(uint8_t));
}

void serial_wake_up_gps(GPS_t * gps_t){
  /* Turn ON RF section */
  uint8_t gps_on[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x09, 0x00, 0x17, 0x76};
  serial_send_UBX_gps(gps_t, gps_on, sizeof(gps_on)/sizeof(uint8_t));
  vTaskDelay(100 / portTICK_PERIOD_MS);
}
