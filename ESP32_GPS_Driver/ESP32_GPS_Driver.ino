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

/* Dependancy objects to the GPS_t type */
#ifdef USE_SOFTWARE_SERIAL
  SoftwareSerial serial_port; // Make sure to configure the corresponding pins in GPS.h
#else
  HardwareSerial serial_port(2); // Make sure to configure the corresponding pins in GPS.h ({RX, TX} :: Serial0 -> 03, 01; Serial1 -> 17, 16; Serial2 -> 27, 26)
#endif // USE_SOFTWARE_SERIAL

TinyGPSPlus gps;

/* Our GPS_t object */
GPS_t my_gps;

/* Data containers */
LOCATION_t current_location;
LOCATION_t EIFFEL_LOC;
DATETIME_t DATETIME;

/* Globals */
float speed_;
float distance_;
uint8_t err_code;
bool one_time = false, gps_good = false;

void interpret_read_error(uint8_t err_code, char *buf);

/*-----------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(115200);

  /* Initialize the gps connection */
  serial_init_gps(&my_gps, &gps, &serial_port);
}

void loop() {

  uint32_t time_now = millis();

  /* Printing buffer */
  char buf[255];
  
  #define READ_FREQUENCY      100U // ms --> 10Hz
  static uint32_t last_read;
  if(time_now - last_read > READ_FREQUENCY){
    
    /* Read & interpret the data */
    err_code = serial_read_gps(&my_gps, &current_location, &DATETIME, &speed_);
    
    /* You can choose which data to read:
     * 
     * NO SPEED: 
     *    err_code = serial_read_gps(&my_gps, &current_location, &DATETIME, NULL);
     * NO DATE: 
     *    err_code = serial_read_gps(&my_gps, &current_location, NULL, &speed_);
     */
     
    interpret_read_error(err_code, buf);
    
    last_read = time_now;
  }

  /* Only print once a second */
  #define PRINT_FREQUENCY     1000U // ms
  static uint32_t last_print;
  if(time_now - last_print > PRINT_FREQUENCY){
    Serial.println(buf);
    last_print = time_now;
  }
  
  /* Calculate distance to the Eiffel tower only once (first time data is valid) */
  if(one_time){
    /* Coordinates to the Eiffel tower */
    EIFFEL_LOC = {
      .lat = 48.85826,
      .lon = 2.294516,
    };
    
    serial_distance_from_to_gps(&current_location, &EIFFEL_LOC, &distance_);
    
    sprintf(buf, "### Distance to Eiffel tower: %f[km] ###", distance_/1000.0F);
    Serial.println(buf);

    one_time = false;
  }
}

/*-----------------------------------------------------------------------------------*/

/* Decode the error codes returned by the read function */
void interpret_read_error(uint8_t err_code, char *buf){
  switch(err_code){
    case ERR_WIRING_SERIAL:
      strcpy(buf, "Physical connection error, you might have a loose wire, or you're using the wrong hardware serial port");
      break;

    case ERR_NO_GPS_FIX:
      strcpy(buf, "No fix yet, you either are inside, have a loose antenna, or simply have to wait");
      break;

    case ERR_GPS_DATA_INVALID:
      strcpy(buf, "Non-valid data, you're probably reading the GPS data too slowly, too fast, or unfrequently");
      break;
    
    case ERR_GPS_ALL_GOOD: 

      /* Calculate the distance once */
      if(!gps_good && !one_time) gps_good = one_time = true;
            
      /* Visualize the read data */
      sprintf(buf, "Location: \n"
                     "\tLat: %f, Lon: %f \n"
                   "Date: %d/%d/%d, Time: %d:%d:%d \n"
                   "Speed: %f",
                   current_location.lat, current_location.lon, 
                   DATETIME.yy, DATETIME.mm, DATETIME.dd,
                   DATETIME.hr, DATETIME.mn, DATETIME.sc,
                   speed_);
      break;
  }
}
