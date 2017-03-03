# coding=iso-8859-15
from itom import *
import matplotlib
import matplotlib.pyplot as plt
from matplotlib import rc
import numpy as np

#http://matplotlib.org/users/customizing.html
matplotlib.rcParams['text.latex.unicode'] = True # latex unicode
matplotlib.rcParams['font.family'] = 'serif'
matplotlib.rcParams['text.latex.preamble'] = [r'\usepackage{lmodern}']
rc('text', usetex=True)

#########################################################################
## plot
targetPDFname = 'texDemo'
fileFormat = 'pdf'

(value, accepted) = ui.getItem('Matplotlib text rendering', 'please select', ['default', 'latex', 'latex unicode'], currentIndex=2, editable=False)

if value == 'default': #default
    matplotlib.rcParams['text.usetex'] = False
    matplotlib.rcParams['text.latex.unicode'] = False
    title = 'TeX is Number $\sum_{n=1}^\infty \frac{-e^{i\pi}}{2^n}$!'
    xLabel = 'time [s]'
    yLabel = 'velocity [�/sec]'
    
elif value == 'latex':
    matplotlib.rcParams['text.usetex'] = True
    matplotlib.rcParams['text.latex.unicode'] = False
    title = r'\TeX\ is Number $\sum\limits_{n=1}^\infty \frac{-e^{i\pi}}{2^n}$!'
    xLabel = r'time [s]'
    yLabel = r'\textit{velocity [\ensuremath{^\circ}/sec]}'
    
elif value == 'latex unicode':
    matplotlib.rcParams['text.usetex'] = True
    matplotlib.rcParams['text.latex.unicode'] = True
    title = r'\TeX\ is Number $\sum\limits_{n=1}^\infty \frac{-e^{i\pi}}{2^n}$!'
    xLabel = r'\textbf{time [s]}'
    yLabel = u'\\textit{velocity [\u00B0/sec]}'
    

clc() # clear command line
plt.cla() # clear figure
plt.close('all') # close figures

plt.figure(1, figsize=(6.29921, 5)) # create a new figure window
#ax = plt.axes([0.1, 0.1, 0.8, 0.7]) 
t = np.arange(0.0, 1.0 + 0.01, 0.01)
s = np.cos(2*2*np.pi*t) + 2

plt.plot(t, s) # plot line

plt.xlabel(xLabel) # x axis label
plt.ylabel(yLabel) # y axis label
plt.title(title, fontsize=16, color='r') # title

plt.grid(True) # create grid

spaceToBorder = 0.15
plt.subplots_adjust(left=spaceToBorder, right=1- spaceToBorder, top=1 - spaceToBorder, bottom=spaceToBorder) # adjust the space to the border of the figure

plt.show()# show the plot

#get current figure
current_figure = plt.gcf()

#set the keepSizeFixed property of the plot to true:
current_figure.canvas.manager.itomUI["keepSizeFixed"] = True

#change the size
current_figure.set_dpi(120)
current_figure.set_size_inches(6.29921, 5, forward = True) # 6.29921 inches are 16mm width of a DIN A4 page
    
plt.savefig(targetPDFname + "." + fileFormat, format = fileFormat)