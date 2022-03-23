# Maintainer document

## Maintainer guide

## Test plan

1. Hub reconnection
  - WHAT: Hub connects to watch, watch turns off and on, Hub is able to automatically reconnect to watch
  - EXPECTED RESULT: The hub reconnected and can save a new session

The goal of this document is to provide a comprehensive test plan for the maintainer and development team to perform the following opperations:

- flash the firmware to the watch 
- perform firmware updates to the watch
- updating the system to the hub with new software
    - rebooting hub -> ssh in -> pulling github repo -> starting new software
- test basic functionalities of the system
