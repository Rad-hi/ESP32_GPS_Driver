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
 
  #include <HardwareSerial.h>
  #include <TinyGPS++.h>

  /* GPS config params */
  #define GPS_SER_RX              26
  #define GPS_SER_TX              27        
  #define GPS_BAUD_RATE           9600      

  /* GPS error codes */
  #define ERR_GPS_DATA_INVALID    0
  #define ERR_NO_GPS_FIX          1
  #define ERR_WIRING_SERIAL       2
  #define ERR_GPS_ALL_GOOD        3
  
  /* Config container */
  typedef struct{
    HardwareSerial * serial_gps_handle;
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
  void serial_init_gps(GPS_t * gps_t, TinyGPSPlus * gps_, HardwareSerial * serial_port);
  
  uint8_t serial_read_gps(GPS_t * gps_t, LOCATION_t * loc, DATETIME_t * datetime, float * distance);
  
  void serial_distance_from_to_gps(LOCATION_t * from_, LOCATION_t * to_, float * distance);

  void serial_put_to_sleep_gps(GPS_t * gps_t);
  void serial_wake_up_gps(GPS_t * gps_t);
  
#endif // __GPS_H__
