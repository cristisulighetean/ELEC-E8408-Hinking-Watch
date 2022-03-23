# ELEC-E8408-Hiking-Watch
Mini project from the ELEC-E8408 course, Aalto University. which consists of a TTGO-Watch-2020-V2 and a companion computer (Raspberry Pi)

TODO explain project

## Installation

### Hub

#### Dependencies

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

### Watch
TODO Cristian

## Supported hardware
TODO Cristian

## Project layout


## Maintainers
Oliver Lagerroos  
Simon Nordström  
Gabor Pelesz  
Cristian Sulighetean  