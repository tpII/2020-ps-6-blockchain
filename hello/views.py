import requests

from django.shortcuts import render
from django.http import HttpResponse

from .models import M

# Create your views here.
def index(request):
    return render(request, "index.html")

def cards(request):
    context = {'cardKeyss': M.cardKeys}
    return render(request, "cards.html", context)
    
def cards_add(request):
    postedcardkey = request.GET['cardkey']
    keylocation = request.GET['location']
    M.addCard(postedcardkey,keylocation)
    ms = zip(M.locations,M.cardKeys) ##list of lists 0,1
    context = {'listss': ms}
    return render(request, "cards.html", context)

def cards_clear(request):
    M.clear()
    return render(request, "cards.html")

