# ELEC-E8408-Hinking-Watch-
Mini project from the ELEC-E8408 course which consists of a TTGO-Watch-2020-V2 and a companion computer (Raspberry Pi)


# Bluetooth communication between the Hub and the Watch 
## MAC addresses
Watch: `44:17:93:88:D1:D2`
Hub: `B8:27:EB:64:86:8D`

## Pairing
When the Hub connects successfully to the Watch, it will send one `c` (ASCII character) to notify the Watch of the connection.

## Synchronization data
After pairing, the watch
```python
# id;steps;km;lat1,long1;lat2,long2;...\r\n
b'4;2425;324;64.83458747762428,24.83458747762428;...,...\r\n'
```
- The carriage return and the newline represents the end of the session synchronization message

After the Hub successfully receives the session data the Hub sends one `r` (ASCII character) that represents *received*