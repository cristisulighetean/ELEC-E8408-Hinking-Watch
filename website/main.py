from flask import Flask
from flask import render_template
from flask import jsonify


app = Flask(__name__)

@app.route('/')
def url():
    return render_template('home.html')

@app.route('/sessions')
def search_options():
    timestamp1 = "19-03-2022 06:50"
    timestamp2 = "19-03-2022 08:20"
    timestamp3 = "19-03-2022 09:35"
    timestamp_list = []
    timestamp_list.append(timestamp1)
    timestamp_list.append(timestamp2)
    timestamp_list.append(timestamp3)
    return jsonify(timestamp_list)


@app.route('/sessions/<id>')
def search_parameters(id):
    parameters = ["2098", "1.34", "84"]
    return jsonify(parameters)

if __name__ == "__main__":
    app.run(debug=True)