# ELEC-E8408-Hiking-Watch
Mini project from the ELEC-E8408 course, Aalto University. which consists of a TTGO-Watch-2020-V2 and a companion computer (Raspberry Pi)

The project is a system consisting of a smart watch (Watch) application that is able to record and display hiking trip statistics, and external device (Hub) that is able to synchronize hiking session data from the watch to store it, manage it, and display the history. The Watch and the Hub is communicating via Bluetooth by a very simple protocol.

With the Watch, the user is able to record hiking sessions, observe the current session data (step count, distance in km), and save one session locally.

The Hub will automatically synchronize to the Watch if they are nearby each other. The incoming sessions will be stored on the Hub inside a database. The Hub serves a website through which the stored session data can be observed, and individual sessions can be deleted. On top of the Hub there is an LED matrix which displays the latest session.

![context_diagram](docs/images/context_diagram.png)

## Hub installation guide

### Dependencies

**Pybluez**

In order to install `pybluez` on an RPi, you have to install some of its dependencies.

```sh
sudo apt-get install libbluetooth-dev
sudo apt-get install python-dev
sudo pip install PyBluez
```

**UnicornhatHD python library**

To make the LED hat work, you have to install the `unicornhathd` python library. The library has to be installed as a super user.

`sudo pip3 install unicornhathd==0.0.4`

**Other**

The required python libraries can be installed from the `hub/requirements.txt` file.
Assuming you are in the root of the project, run:

`pip3 install -r hub/requirements.txt`

### Installing the Hub

```bash
git clones https://github.com/cristisulighetean/ELEC-E8408-Hinking-Watch.git

cd ELEC-E8408-Hinking-Watch/hub

sudo sh starthub.sh
```

---

## Watch installation guide

The development process of the watch firmware we recommend to be performed using PlatformIO. After opening the `watch_fw` as a project, make sure that the `TTGO Watch` library is added.

To develop and flash firmware for the watch, we will need the 
 following elements:

    - Hardware: 
        - Good quality USB Cable
        - Raspberry Pi 3B+ or a computer with at least similar performance 
    - Software:
        - esptool (v3.3)

### Flashing firmware to the watch
    
The firmware update will be published on GitHub as a binary file, and it can be flashed to the watch by using the following command:

```sh
esptool.py --chip esp32 \
	--port <PORT> \
	--baud 2000000 \
	--before default_reset \
	--after hard_reset write_flash \
	-z --flash_mode dio \
	--flash_freq 80m \
	--flash_size detect \
	0x1000 bootloader_dio_80m.bin \
	0x8000 partitions.bin \
	0xe000 boot_app0.bin \
	0x10000 firmware.bin
```

The remaining `bootloader_dio_80m.bin`, `boot_app0.bin` and `partitions.bin` files can be found under the `misc` folder in watch firmware.

**Other** 

To flash your own binary, you could also use Platform IO and install the `TTGO watch` library, while selecting the `TTGO watch` as the board

## Supported hardware

### Hub 

The hub can be any Raspberry device starting with the Pi 3
- the software was only tested on a Pi 3B+

### LED Display

We are using the Unicorn Hat from Pimoroni, which fits directly to the Raspberry Pi pin header.

### Watch 

At the moment we only support the TTGO V2 2020 smartwatch. The V2 is only model which is supplied with the GPS module, and the main hiking fuctionality is based on it.

It can be bought from either [aliexpress](aliexpress.com) or from the manufacutrer's website [lilygo](liltgo.cn)

## Project layout


## Maintainers
Oliver Lagerroos  
Simon Nordström  
Gabor Pelesz  
Cristian Sulighetean  