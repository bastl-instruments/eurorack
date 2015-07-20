#! /usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
from scipy import interpolate


def smooth(x,y):

	tck = interpolate.splrep(x, y, s=0)
	xnew = np.arange(0,256,1)
	ynew = interpolate.splev(xnew, tck, der=0)


	plt.figure()
	plt.plot(x, y, 'x', xnew, ynew, x, y, 'b')
	plt.legend(['Linear', 'Smoothed'])
	#plt.axis([-0.05, 6.33, -1.05, 1.05])
	plt.title('Smoothed Curve')
	plt.show()



def interpolate(points):

	res = []
	afterNumb = 0
	for y in range(0,256):

		if y>points[afterNumb+1][0]:
			afterNumb = afterNumb +1
		
		dx = points[afterNumb+1][0]-points[afterNumb][0]
		dy = points[afterNumb+1][1]-points[afterNumb][1]
	
		res.append(int(round(points[afterNumb][1] + (y-points[afterNumb][0])*dy/dx)))
	

	print res
