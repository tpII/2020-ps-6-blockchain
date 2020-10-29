from django.db import models

# Create your models here.
class M(models.Model):
    cardKeys = []
    when = models.DateTimeField("date created", auto_now_add=True)
