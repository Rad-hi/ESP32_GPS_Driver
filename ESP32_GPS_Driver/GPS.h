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

#ifndef __GPS_H__
#define __GPS_H__

  #define USE_SOFTWARE_SERIAL

  #ifdef USE_SOFTWARE_SERIAL
    #include <SoftwareSerial.h> // https://github.com/plerup/espsoftwareserial
    #define INVERT_LOGIC            false // https://github.com/plerup/espsoftwareserial/blob/eb4b29074b75eacac3585bf84b5495b8f80a92cf/src/SoftwareSerial.h#L97
    #define BUF_SIZE                256
    #define GPS_SOFT_SER_RX         27
    #define GPS_SOFT_SER_TX         26
  #else
    #include <HardwareSerial.h>
    #define GPS_SER_RX              27
    #define GPS_SER_TX              26
  #endif // USE_SOFTWARE_SERIAL
  
  #include <TinyGPS++.h>

  /* GPS config params */
  #define GPS_BAUD_RATE           9600      

  #define GPS_READ_FREQUENCY      200U // ms
  /* GPS error codes */
  #define ERR_GPS_DATA_INVALID    0
  #define ERR_NO_GPS_FIX          1
  #define ERR_WIRING_SERIAL       2
  #define ERR_GPS_ALL_GOOD        3
  
  /* Config container */
  typedef struct{
    #ifdef USE_SOFTWARE_SERIAL
      SoftwareSerial * serial_gps_handle;
    #else
      HardwareSerial * serial_gps_handle;
    #endif // USE_SOFTWARE_SERIAL
    
    TinyGPSPlus * gps;
  }GPS_t;

  /* Data container for locations */
  typedef struct{
    float lat;
    float lon;
  }LOCATION_t;

  /* Data container for the date & time */
  typedef struct{
    /* Date data */
    uint8_t dd;
    uint8_t mm;
    uint16_t yy;

    /* Time data */
    uint8_t sc;
    uint8_t mn;
    uint8_t hr;
  }DATETIME_t;

  /* Functions */
  void serial_init_gps(GPS_t * gps_t, 
                       TinyGPSPlus * gps_, 
                       #ifdef USE_SOFTWARE_SERIAL
                         SoftwareSerial * serial_port
                       #else
                         HardwareSerial * serial_port
                       #endif // USE_SOFTWARE_SERIAL
                       );
  
  uint8_t serial_read_gps(LOCATION_t * loc, DATETIME_t * datetime, float * distance);
  void serial_distance_from_to_gps(LOCATION_t * from_, LOCATION_t * to_, float * distance);
  
  void serial_put_to_sleep_gps();
  void serial_wake_up_gps();
  
#endif // __GPS_H__
