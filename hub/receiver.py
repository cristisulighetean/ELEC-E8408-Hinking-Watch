import time
import sqlite3

import hike
import db
import bt

hubdb = db.HubDatabase()
hubbt = bt.HubBluetooth()

def process_sessions(sessions: list[hike.HikeSession]):
    for s in sessions:
        s.calc_kcal()
        hubdb.save(s)

def main():
    try:
        while True:
            hubbt.wait_for_connection()
            hubbt.synchronize(callback=process_sessions)
    except Exception as e:
        print(e)
        hubbt.sock.close()

if __name__ == "__main__":
    main()