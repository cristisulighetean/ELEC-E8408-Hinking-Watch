import bluetooth
import time

import hike

class HubBluetooth:
    WATCH_BT_MAC = '44:17:93:88:D1:D2'
    WATCH_BT_PORT = 700
    sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    connected = False
    
    def wait_for_connection(self):
        if not self.connected:
            # try to connect every sec while connection is made
            while True:
                try:
                    self.sock.connect((self.WATCH_BT_MAC, self.WATCH_BT_PORT))
                    self.connected = True
                    break
                except bluetooth.btcommon.BluetoothError:
                    time.sleep(1)
                except Exception as e:
                    print(e)
                    print("Hub: Error occured while trying to connect to the Watch.")

            print("Hub: Established Bluetooth connection with Watch!")
        print("WARNING Hub: the has already connected via Bluetooth.")

    def synchronize(self, callback):
        try:
            remainder = b''
            while True:
                chunk = self.sock.recv(1024)

                messages = chunk.split(b'\n')
                messages[0] = remainder + messages[0]
                remainder = messages.pop()

                sessions = HubBluetooth.messages_to_sessions(messages)
                callback(sessions)
        except KeyboardInterrupt:
            raise Exception("Shutting down server.")
        except:
            self.connected = False
            # TODO handle when connection goes down
        finally:
            self.sock.close()

    @staticmethod
    def messages_to_sessions(messages: list[bytes]) -> list[hike.HikeSession]:
        def mtos(m: bytes) -> hike.HikeSession:
            # id;steps;km;lat1,long1;lat2,long2;...\r\n
            # b'4;2425;324;64.83458747762428,24.83458747762428;...,...\r\n'
            m = m.decode('utf-8')

            parts = m.split(';')
            assert len(parts) >= 4, f"MessageProcessingError -> The incoming message doesn't contain enough information: {m}"

            hs = hike.HikeSession()
            hs.id     = int(parts[0])
            hs.steps  = int(parts[1])
            hs.km     = int(parts[2])

            def cvt_coord(c):
                sc = c.split(',')
                assert len(sc) == 2, f"MessageProcessingError -> Unable to process coordinate: {c}"
                return float(sc[0]), float(sc[1])
                
            hs.coords = map(cvt_coord, parts[3:])

            return hs

        return list(map(mtos, messages))

