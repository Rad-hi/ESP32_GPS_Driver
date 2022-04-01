# Notes

## Link: github.com/Rad-hi/ESP32_GPS_Driver

### 1- Wiring:

- Hardware Serial 

    --> Serial 0 (Serial)  : Rx 03, Tx 01
    --> Serial 1 (Serial1) : Rx 17, Tx 16
    --> Serial 2 (Serial2) : Rx 27, Tx 26
	
- Software Serial

    --> Any compatible pair of pins (some pins are input/output only !)
		

		ESP32		GPS 
	GND		<-->		GND
	Tx		<-->		Rx
	Rx		<-->		Tx
	3.3V		<-->		VCC

### 2- Read frequency:

    Around 10Hz (stick to 5-8 Hz)

### 3- Operation medium:

    Outdoor, Good weather.

### 4- Extra notes:

    Cold start (up to 30m), be patient!
    Antenna quality matters.
    AGPS (Assisted GPS) for improved accuracy, and faster starting times.
