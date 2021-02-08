from flask import Flask, jsonify, request
from flask_sqlalchemy import SQLAlchemy

#DATABASE
db = SQLAlchemy() #our database

#TABLES
class Block(db.Model):
    __tablename__= 'block'
    index = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.String(200))
    proof = db.Column(db.Integer)
    previous_hash = db.Column(db.String(200))
    transactions = db.relationship("Transaction")

    def __init__(self, index, timestamp, proof, previous_hash):
        self.index = index
        self.timestamp = timestamp
        self.proof = proof
        self.previous_hash = previous_hash

class Transaction(db.Model):
    __tablename__= 'transaction'
    id = db.Column(db.Integer, primary_key=True)
    sender = db.Column(db.String(200))
    recipient = db.Column(db.String(200))
    amount = db.Column(db.Integer)
    cardkey = db.Column(db.String(200))
    location = db.Column(db.String(200))
    date = db.Column(db.String(200))
    index = db.Column(db.Integer, db.ForeignKey('block.index'))

    def __init__(self, sender, recipient, amount, cardkey, location, date, index):
        self.sender = sender
        self.recipient = recipient
        self.amount = amount
        self.cardkey = cardkey
        self.location = location
        self.date = date
        self.index = index



