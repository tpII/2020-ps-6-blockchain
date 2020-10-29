import requests

from django.shortcuts import render
from django.http import HttpResponse

from .models import Greeting


# Create your views here.
def index(request):
    return render(request, "index.html")

def cards(request):
    return render(request, "cards.html")
    
def cardsadd(request):
    postedcardkey = request.GET['cardkey']
    context = {'cardkey': postedcardkey}
    return render(request, "cards.html", context)
