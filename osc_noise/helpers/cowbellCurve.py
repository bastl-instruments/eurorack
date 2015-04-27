#! /usr/bin/python

from curveCalculator import *

relation = float(293)/377
maxHigh = 800

high=[]

high.append((0,10))
high.append((128,138))
high.append((255,maxHigh))

interpolate(high)

low=[]
low.append((0,8))
low.append((128,int(138*relation)))
low.append((255,int(maxHigh*relation)))

interpolate(low)







