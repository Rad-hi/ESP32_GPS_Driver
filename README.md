# GPS driver for the NEO6M GPS module that works with the ESP32 dev board

This is a simple wrapper for the TinyGPS++ library that's intended to work with an ESP32.

## Implemented functionalities

Data reading function were provided, as well as a function to calculate the distance between two locations. Put and wake-up from sleep functions were implemented as well. 

The available data is:

- Latitude, Longitude.

- Speed [Km/h].

- Date and Time.

Only functions that I need are implemented, but many more could be depending on the need, but the current API allows for expansion.

## Compatibility

This code is only tested on an ESP32, ~but it would probably work with an Arduino UNO (or any other one) given that the Serial port is appropriately configured~ an ROTS task was created for asynchronous data parsing, hence the prohibition of compatibility with any Arduino board.

Implemented SoftwareSerial communication support for ease of portability for boards with less hardware Serial ports.

## How it works

A timer interrupt notifies a blocked task that it could parse data (the notify frequency is configurable), and on each period, a local copy of the data is updated with the valid read data, which accessible to the main task asynchronously (non dependent from reading).
