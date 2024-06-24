from flask import Flask, render_template, request, jsonify
import os
import json


app = Flask(__name__) #creating the Flask class object

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/putdata', methods=['POST'])
def putdata():
    data = request.files
    request.files["fileIntegr"].save("run/IntegrVelocity.xlsx")
    request.files["fileGraph"].save("run/Graph.xlsx")
    request.files["fileShips"].save("run/Ships.xlsx")

    os.system("cd run && python3 create_graph.py")
    os.system("cd run && ./scheduler " + "Graph.xlsx " + "Ships.xlsx " + request.args.get("car"))
    os.system("cd run && python3 totime.py")

    f = open('run/schedule.json')

    return f

if __name__ =='__main__':
    app.run(host="0.0.0.0", port = 8111)