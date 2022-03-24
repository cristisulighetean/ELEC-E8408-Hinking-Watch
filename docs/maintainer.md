# Maintainer document

## Maintainer guide

### Flashing firmware to the watch
To perform firmware updates to the watch the maintainer will need the following elements:

    - Hardware: 
        - Good quality USB Cable
        - Raspberry Pi 3B+ or a computer with at least similar performance 
    - Software:
        - esptool (v3.3)
    
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
### Perform software updates to the Hub

To perform software updates to the Hub, the maintainer will need a computer connected to the same local network as the Hub. The local network must also be connected to the internet to be able to download the software updates.

#### Connecting to the Hub
The maintainer can connect to the Hub via the `ssh` service using the credentials provided by the user.

After connecting via `ssh`, the maintainer needs to stop by rebooting the Hub with 
    
    sudo reboot now

And then reconnect via `ssh` and perform the following commands

    cd ELEC-E8408-Hinking-Watch-/hub
    git pull
    chmod +x starthub.sh
    sudo ./starthub.sh

After performing the previous step, the maintainer can end the `ssh` session by typing `exit` in the terminal.

## Test plan

1. Hub reconnection
    - `GOAL`: Test how the Hub will handle a reconnection if the watch is turned off by mistake, or the battery runs out in the watch
    - `HOW`: Wait for the Hub to connect to the watch, turn the watch off and on
    - `EXPECTED RESULT`: The hub reconnected and can save a new session

2. Sudden Hub restart due to power outage
    - `GOAL`: Test if the Hub will restart the processes after a power outage
    - `HOW`: Unplug the power supply of the Hub, and then plug it back again. Test the webpage and sync a hike session. 
    - `EXPECTED RESULT`: The user will be able to access the webpage, and also sync hiking sessions to the Hub

3. Watch sending session without having GPS data
    - `GOAL`: Test how the Hub will handle receiving a session that has no coordinate data
    - `HOW`: Sync a hiking session that has not been able to record any coordinates. This can be done by recording a quick session while being inside a building. It is hard for the GPS signal to pass through the walls, so it is likely that the watch will not be able to establish any communication with the GPS satellites, and record coordinates.
    - `EXPECTED RESULT`: The Hub will handle syncing the session, but it will show the distance parameter at 0 Km.

4. Watch disconnecting while sending data to the Hub
    - `GOAL`: Test if the watch will send the stored session again, if it was disconnected while sending it
    - `HOW`: Take out the watch battery while it says that the watch is sending the data
    - `EXPECTED RESULT`: The watch should send the stored session once it is powered up

5. Watch showing no GPS data while recording session
    - `GOAL`: Testing whether the GPS is unable to retrieve position.
    - `HOW`: Start a hiking session while being outside and record 20 minutes of hiking.
    - `EXPECTED RESULT`: If during the 20 minutes hiking session, the watch is unable to retrieve GPS position, restart the watch.

6. Check if last stored session is shown on the LED display
    - `GOAL`: Test whether the last stored session is shown on the LED display
    - `HOW`: Record a hiking session, and the sync it to the watch.
    - `EXPECTED RESULT`: After the syncing procedure has been completed, the LED display should show the last hiking session
