import hashlib
import json
import time
import datetime
from textwrap import dedent
from uuid import uuid4
import sys #args
from models.Blockchain import *
from models.Cardkey import *
from flask import Flask, jsonify, request, render_template

#args port
#if (len(sys.argv) == 3):
#    myport = int(sys.argv[1])
#else:
#    myport = 80
myport = 80
myhost = "ps6taller.herokuapp.com"

# Instantiate our Node
app = Flask(__name__)

# Generate a globally unique address for this node
node_identifier = str(uuid4()).replace('-', '')

# Instantiate the Blockchain
blockchain = Blockchain()

@app.context_processor
def inject_uuid():
    context = {'nodeuuid': node_identifier}
    return context

#alternative
#@app.route(all)
#def base():
#    test = 'Avaible to all'
#    return render_template('base.html', test=test)

@app.route('/', methods=['GET'])
def index():
    return render_template("index.html")

@app.route('/cards', methods=['GET'])
def cards():
    cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
    context = {'listss': cardsdata}
    return render_template("cards.html", **context)

@app.route('/cards/add/', methods=['GET'])
def cards_add():
    if (request.args.get('cardkey') and request.args.get('location')):
        postedcardkey = request.args.get('cardkey')
        keylocation = request.args.get('location')
        if (postedcardkey) is not None:
            M.addCard(postedcardkey,keylocation)
            cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
            context = {'listss': cardsdata}
            return render_template("cards.html", **context)
        else:
            cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
            context = {'listss': cardsdata}
            return render_template("cards.html",**context)
    else:
        cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
        context = {'listss': cardsdata}
        return render_template("cards.html", **context)
 
@app.route('/cards/clear', methods=['GET'])
def cards_clear():
    M.clear()
    return render_template("cards.html")

@app.route('/chain', methods=['GET'])
def full_chain():
    response = {
        'chain': blockchain.chain,
        'length': len(blockchain.chain),
    }
    return jsonify(response), 200

# add tx enpoinsd
@app.route('/transactions/new', methods=['POST'])
def new_transaction():
    values = request.get_json()

    # Check that the required fields are in the POST'ed data
    required = ['sender', 'recipient', 'amount', 'cardkey', 'location', 'date']
    if not all(k in values for k in required):
        return 'Missing values', 400

    # Create a new Transaction
    index = blockchain.new_transaction(
        values['sender'], values['recipient'], values['amount'], values['cardkey'], values['location'], values['date'])

    response = {'message': f'Transaction will be added to Block {index}'}
    return jsonify(response), 201

# mining endpoint
# Calculate the Proof of Work
# Reward the miner (us) by adding a transaction granting us 1 coin
# Forge the new Block by adding it to the chain


@app.route('/mine', methods=['GET'])
def mine():
    # We run the proof of work algorithm to get the next proof...
    last_block = blockchain.last_block
    last_proof = last_block['proof']
    proof = blockchain.proof_of_work(last_proof)

    # We must receive a reward for finding the proof.
    # The sender is "0" to signify that this node has mined a new coin.
    blockchain.new_transaction(
        sender="0",
        recipient=node_identifier,
        amount=0,  #no reward
        cardkey=0,
        location=0,
        date=str(datetime.datetime.now()).split('.')[0], #unix time
    )

    # Forge the new Block by adding it to the chain
    previous_hash = blockchain.hash(last_block)
    block = blockchain.new_block(proof, previous_hash)

    response = {
        'message': "New Block Forged",
        'index': block['index'],
        'transactions': block['transactions'],
        'proof': block['proof'],
        'previous_hash': block['previous_hash'],
    }
    return jsonify(response), 200
#nodes
@app.route('/nodes/register', methods=['POST'])
def register_nodes():
    values = request.get_json()

    nodes = values.get('nodes')
    if nodes is None:
        return "Error: Please supply a valid list of nodes", 400

    for node in nodes:
        blockchain.register_node(node)

    response = {
        'message': 'New nodes have been added',
        'total_nodes': list(blockchain.nodes),
    }
    return jsonify(response), 201


@app.route('/nodes/resolve', methods=['GET'])
def consensus():
    replaced = blockchain.resolve_conflicts()

    if replaced:
        response = {
            'message': 'Our chain was replaced',
            'new_chain': blockchain.chain
        }
    else:
        response = {
            'message': 'Our chain is authoritative',
            'chain': blockchain.chain
        }

    return jsonify(response), 200

if __name__ == '__main__':
    app.run(host=myhost, port=myport)
	


    

