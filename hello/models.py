from django.db import models

# Create your models here.
class M(models.Model):
    cardKeys = []
    locations = []
    len = 0

    def getContext():
        context['cardKeyss'] = M.cardKeys
        context['locationss'] = M.locations
        context['lenn'] = M.len
        return context

    def addCard(key,loc):
        M.cardKeys.append(key)
        M.locations.append(loc)
        M.len+=1

    def clear():
        M.cardKeys = []
        M.locations = []
        M.len = 0

    when = models.DateTimeField("date created", auto_now_add=True)
