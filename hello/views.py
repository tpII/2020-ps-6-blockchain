import requests

from django.shortcuts import render
from django.http import HttpResponse

from .models import M

# Create your views here.
def index(request):
    return render(request, "index.html")

def cards(request):
    cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
    context = {'listss': cardsdata}
    return render(request, "cards.html", context)
    
def cards_add(request):
    if (request.method == 'GET' and 'cardkey' in request.GET and 'location' in request.GET):
        postedcardkey = request.GET['cardkey']
        keylocation = request.GET['location']
        if (postedcardkey) is not None:
            M.addCard(postedcardkey,keylocation)
            cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
            context = {'listss': cardsdata}
            return render(request, "cards.html", context)
        else:
            cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
            context = {'listss': cardsdata}
            return render(request, "cards.html", context)
    else:
        cardsdata = zip(M.locations,M.cardKeys) ##list of lists 0,1
        context = {'listss': cardsdata}
        return render(request, "cards.html", context)
        

def cards_clear(request):
    M.clear()
    return render(request, "cards.html")

