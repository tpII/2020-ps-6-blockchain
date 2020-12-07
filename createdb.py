from models.db import *
from flask import Flask, jsonify, request, render_template
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
#app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://admin:admin@localhost/ps6db' #db config p://user:password@url/dbname
app.config['SQLALCHEMY_DATABASE_URI'] = 'postgres://imnffvyhuzlzqz:464bfb6d496419662ea566e42c3b36df6eb2ce12fd125ed3a3234d3974a9026c@ec2-107-20-104-234.compute-1.amazonaws.com:5432/dprs7j65nogo9'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db.init_app(app)

with app.app_context():
    db.create_all()

