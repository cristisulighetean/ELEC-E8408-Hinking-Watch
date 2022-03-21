import sqlite3

import hike

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

class HubDatabase:
    def __init__(self):
        self.con = sqlite3.connect('sessions.db')
        self.cur = self.con.cursor()

        for t in (DB_SESSION_TABLE, DB_COORDINATE_TABLE):
            create_table_sql = f"create table if not exists {t['name']} ({', '.join(t['cols'])})"
            self.cur.execute(create_table_sql)
        self.con.commit()

    def save(self, s: hike.HikeSession):
        self.cur.execute(f"INSERT INTO {DB_SESSION_TABLE['name']} VALUES ({s.id}, {s.km}, {s.steps}, {s.kcal})")
        
        for c in s.coords:
            self.cur.execute(f"INSERT INTO {DB_COORDINATE_TABLE['name']} VALUES ({s.id}, {c[0]}, {c[1]})")

        self.con.commit()

    def delete(self, session_id):
        self.cur.execute(f"DELETE FROM {DB_COORDINATE_TABLE['name']} WHERE session_id = {session_id}")
        self.cur.execute(f"DELETE FROM {DB_SESSION_TABLE['name']} WHERE session_id = {session_id}")
        self.con.commit()

    def get_sessions(self) -> list[hike.HikeSession]:
        rows = self.cur.execute(f"SELECT * FROM {DB_SESSION_TABLE['name']}").fetchall()
        return list(map(lambda r: hike.from_list(r), rows))

    def get_coordinates(self, session_id):
        return self.cur.execute(f"SELECT lat, long FROM {DB_COORDINATE_TABLE['name']} WHERE session_id = {session_id}").fetchall()

    def __del__(self):
        self.cur.close()
        self.con.close()