# coding=iso-8859-15
''' This demo shows how to display contour lines in an itom2dQwtPlot.

The contour levels can be set via the contourLevels property. The expected type is a dataObject of any shape of type uint8, int8, uint16, int16, int32,
float32 or float64. The line width and color map can be adjusted setting the property contourLineWidth and contourColorMap respectively.'''

import numpy as np
vec=np.linspace(-500,501,1001)
x,y = np.meshgrid(vec,vec)
r= np.sqrt(x**2+y**2)

pl = plot(r)
levels=dataObject.randN([2,1,2,2],'uint8')

pl[1]['contourLevels']=levels
pl[1]['contourColorMap']='hotIron'
pl[1]['contourLineWidth']=5