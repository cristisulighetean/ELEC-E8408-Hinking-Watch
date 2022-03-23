# Manual

The goal of this document is to familiarize the user with the system and to help with the setup process.
The first part of the document will describe the steps that the user is required to accomplish in order for the system to be up and running. The second part will depict the maintainer's operations on the system.

## General description

The system's goal is to record hiking sessions on the TTGO SmartWatch and synchronize them with the Raspberry Pi so that the user can analyze them afterwards.

## Components of the system

The system is comprised by the following components:

- TTGO SmartWatch V2
- Raspberry Pi 3B+
- Pimoroni LED Display

### Elements needed for the system

Both the user and maintainer will need a computer with an internet connection and a LAN cable to connect the Raspberry Pi to the local network.

## User manual for the user

Note: The Raspberry Pi and Hub terminology has been used interchangeably.

### Setting up the Hub

1. Connect the Raspberry Pi to the local network via an Ethernet cable
1. 1 It is recommended to use an Ethernet cable of category 5 or better
2. Connect the Raspberry Pi to power via the provided power supply
3. Use a tool like [fing](https://www.fing.com/) to find the local IP address of the Raspberry Pi
4. Access the webpage by using [Google Chrome](https://www.google.com/chrome/) and type the local IP address of the Raspberry Pi and the ':5000' at the end
4.1 e.g. '192.168.1.110:5000'
5. If the web browser is able to open the webpage, then the setup has been successful

