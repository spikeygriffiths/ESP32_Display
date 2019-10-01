Uses a TTGO ESP32 with 135 x 240 TFT display talking to a RaspberryPi via a socket to get weather and other information
The RPi puts all the info into a dict and sends that as a string to the ESP32, which parses out the items it wants and displays them accordingly
