import sqlite3

import hike
import threading

DB_FILE_NAME = 'sessions.db'

DB_COORDINATE_TABLE = {
    "name": "coordinates",
    "cols": [
        "session_id INTEGER NOT NULL",
        "lat REAL",
        "long REAL",
        "FOREIGN KEY (session_id) REFERENCES sessions (session_id)"
    ]
}

DB_SESSION_TABLE = {
    "name": "sessions",
    "cols": [
        "session_id integer PRIMARY KEY",
        "km integer",
        "steps integer",
        "burnt_kcal integer",
    ]
}

# lock object so multithreaded use of the same
# HubDatabase object 

class HubDatabase:
    """Hiking sesssion database interface class.

    An object of this class enables easy retreival and management of the
    hiking database content. If the database does not exist, the instantiation
    of this class will create the database inside `DB_FILE_NAME` file.
    
    Arguments:
        lock: lock object so multithreaded use of the same HubDatabase object
              is safe. sqlite3 does not allow the same cursor object to be
              used concurrently.
        con: sqlite3 connection object
        cur: sqlite3 cursor object
    """

    lock = threading.Lock()

    def __init__(self):
        self.con = sqlite3.connect(DB_FILE_NAME, check_same_thread=False)
        self.cur = self.con.cursor()

        for t in (DB_SESSION_TABLE, DB_COORDINATE_TABLE):
            create_table_sql = f"create table if not exists {t['name']} ({', '.join(t['cols'])})"
            self.cur.execute(create_table_sql)

        self.con.commit()

    def save(self, s: hike.HikeSession):
        sessions = self.get_sessions()

        if len(sessions) > 0:
            s.id = sorted(sessions, key=lambda sess: sess.id)[-1].id + 1
        else:
            s.id = 1

        try:
            self.lock.acquire()


            try:
                self.cur.execute(f"INSERT INTO {DB_SESSION_TABLE['name']} VALUES ({s.id}, {s.km}, {s.steps}, {s.kcal})")
            except sqlite3.IntegrityError:
                print("WARNING: Session ID already exists in database! Aborting saving current session.")
            
            for c in s.coords:
                self.cur.execute(f"INSERT INTO {DB_COORDINATE_TABLE['name']} VALUES ({s.id}, {c[0]}, {c[1]})")

            self.con.commit()
        finally:
            self.lock.release()

    def delete(self, session_id: int):
        try:
            self.lock.acquire()

            self.cur.execute(f"DELETE FROM {DB_COORDINATE_TABLE['name']} WHERE session_id = {session_id}")
            self.cur.execute(f"DELETE FROM {DB_SESSION_TABLE['name']} WHERE session_id = {session_id}")
            self.con.commit()
        finally:
            self.lock.release()

    def get_sessions(self) -> list[hike.HikeSession]:
        try:
            self.lock.acquire()
            rows = self.cur.execute(f"SELECT * FROM {DB_SESSION_TABLE['name']}").fetchall()
        finally:
            self.lock.release()

        return list(map(lambda r: hike.from_list(r), rows))

    def get_session(self, session_id: int) -> hike.HikeSession:
        try:
            self.lock.acquire()
            rows = self.cur.execute(f"SELECT * FROM {DB_SESSION_TABLE['name']} WHERE session_id = {session_id}").fetchall()
        finally:
            self.lock.release()

        return hike.from_list(rows[0])

    def get_coordinates(self, session_id: int):
        try:
            self.lock.acquire()
            res = self.cur.execute(f"SELECT lat, long FROM {DB_COORDINATE_TABLE['name']} WHERE session_id = {session_id}").fetchall()
        finally:
            self.lock.release()

        return res

    def __del__(self):
        self.cur.close()
        self.con.close()