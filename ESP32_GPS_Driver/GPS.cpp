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
}

/* This function performs the full read of the GPS data */
uint8_t serial_read_gps(GPS_t * gps_t, LOCATION_t * loc, DATETIME_t * datetime, float * speed_){

  /* To ensure that the readings' quality is maintained, call this frequently */
  gps_t->gps->encode(gps_t->serial_gps_handle->read());

  /* No GPS data available -> wiring/serial problem */
  if (gps_t->gps->charsProcessed() < 10){
    return ERR_WIRING_SERIAL;
  }

  /* There's one or more sentences that failed the checksum -> corrupted data */
  if (gps_t->gps->failedChecksum() >= 1){
    return ERR_GPS_DATA_INVALID;
  }

  /* No sentences have fix -> Go outdoor, check the antenna, or simply wait */
  if (gps_t->gps->sentencesWithFix() < 1){
    return ERR_NO_GPS_FIX;
  }

  /* All good --> Decode new data */
  
  if(loc != NULL && gps_t->gps->location.isUpdated()){
    loc->lat = gps_t->gps->location.lat();
    loc->lon = gps_t->gps->location.lng();
  }

  if(datetime != NULL){
    if(gps_t->gps->date.isUpdated()){
      datetime->dd = gps_t->gps->date.day();
      datetime->mm = gps_t->gps->date.month();
      datetime->yy = gps_t->gps->date.year();
    }
  
    if(gps_t->gps->time.isUpdated()){
      datetime->sc = gps_t->gps->time.second();
      datetime->mn = gps_t->gps->time.minute();
      datetime->hr = gps_t->gps->time.hour();
    }
  }

  if(speed_ != NULL && gps_t->gps->speed.isUpdated()){
    *speed_ = gps_t->gps->speed.mps();
  }

  return ERR_GPS_ALL_GOOD;
}

/* 
 * Given two locations (of type trusted_loccation_t), 
 * this function returns the distance between them in meters. 
 */
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

/* Flush the serial buffer */
void serial_clean_buffer_gps(GPS_t * gps_t){
//  gps_t->serial_gps_handle->flush();
  while(gps_t->serial_gps_handle->available()) gps_t->serial_gps_handle->read();
}
