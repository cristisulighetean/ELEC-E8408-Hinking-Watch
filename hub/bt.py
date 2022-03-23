import bluetooth
import time

import hike

WATCH_BT_MAC = '44:17:93:88:D1:D2'
WATCH_BT_PORT = 1

class HubBluetooth:
    connected = False
    sock = None
    
    def wait_for_connection(self):
        if not self.connected:
            # try to connect every sec while connection is made
            while True:
                print("Waiting for connection...")
                try:
                    self.sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
                    self.sock.connect((self.WATCH_BT_MAC, self.WATCH_BT_PORT))
                    self.sock.settimeout(2)
                    self.connected = True
                    self.sock.send('c')
                    print("Connected to Watch!")
                    break
                except bluetooth.btcommon.BluetoothError:
                    time.sleep(1)
                except Exception as e:
                    print(e)
                    print("Hub: Error occured while trying to connect to the Watch.")

            print("Hub: Established Bluetooth connection with Watch!")
        print("WARNING Hub: the has already connected via Bluetooth.")

    def synchronize(self, callback):
        print("Synchronizing with watch...")
        remainder = b''
        while True:
            try:
                chunk = self.sock.recv(1024)
                print(f"Received data: {chunk}")

                messages = chunk.split(b'\n')
                messages[0] = remainder + messages[0]
                remainder = messages.pop()

                if len(messages):
                    try:
                        print(f"received messages: {messages}")
                        sessions = HubBluetooth.messages_to_sessions(messages)
                        print(f"Saved sessions: {sessions}")
                        callback(sessions)
                        self.sock.send('r')
                        print("Incoming session saved. Sent confirmation to Watch!")
                    except (AssertionError, ValueError) as e:
                        print(e)
                        print("Message was corrupted. Aborting...")
            except KeyboardInterrupt:
                raise KeyboardInterrupt("Shutting down server.")
            except bluetooth.btcommon.BluetoothError as bt_err:
                if bt_err.errno == 11: # connection down
                    self.connected = False
                    self.sock.close()
                    print("Lost connection with the watch.")
                    break
                elif bt_err.errno == None: # possibly occured by socket.settimeout
                    self.sock.send('c')
                    print("Reminder has been sent to the Watch about the attempt of the synchronization.")

    @staticmethod
    def messages_to_sessions(messages: list[bytes]) -> list[hike.HikeSession]:
        def mtos(m: bytes) -> hike.HikeSession:
            # id;steps;km;lat1,long1;lat2,long2;...\n
            # b'4;2425;324;64.83458747762428,24.83458747762428;...,...\n'
            m = m.decode('utf-8')

            # filtering because we might have a semi-column at the end of the message, right before the new-line character
            parts = list(filter(lambda p: len(p) > 0, m.split(';')))
            assert len(parts) >= 3, f"MessageProcessingError -> The incoming message doesn't contain enough information: {m}"

            hs = hike.HikeSession()
            hs.id     = int(parts[0])
            hs.steps  = int(parts[1])
            hs.km     = float(parts[2])

            def cvt_coord(c):
                sc = c.split(',')
                assert len(sc) == 2, f"MessageProcessingError -> Unable to process coordinate: {c}"
                return float(sc[0]), float(sc[1])

            if len(parts) > 3:
                hs.coords = map(cvt_coord, parts[3:])

            return hs

        return list(map(mtos, messages))