from flask import Flask
from flask import render_template
from flask import jsonify

import db as database

app = Flask(__name__)
db = database.Database()

@app.route('/')
def url():
    return render_template('home.html')

@app.route('/sessions')
def search_options():
    timestamp_list = db.get_timestamps() 
    return jsonify(timestamp_list)


@app.route('/sessions/<id>')
def search_parameters(id):
    parameters_dict = db.get_session_data(id)
    return jsonify(list(parameters_dict.values()))

if __name__ == "__main__":
    app.run(debug=True)