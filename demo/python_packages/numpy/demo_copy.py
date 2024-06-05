"""Copy
======
"""

import numpy as np

# sphinx_gallery_thumbnail_path = '11_demos/_static/_thumb/demoNumpy.png'

a = np.array([[0, 1, 2, 3], [4, 5, 6, 7], [8, 9, 10, 11]])

###############################################################################
# No new object is created!
b = a
b is a


###############################################################################
# unique identifier of an object
def f(x):
    print(id(x))


id(a)

###############################################################################
f(a)

###############################################################################
# **View and shallow copy**
c = a.view()
c is a

###############################################################################
# c is a view of the data owned by a
c.base is a

###############################################################################
c.flags.owndata

###############################################################################
# shape of a doesn't change
c = c.reshape((2, 6))

###############################################################################
# a's data changes
c[0, 4] = 1234
a

###############################################################################
# Slicing an array returns a view of it
s = a[:, 1:3]
s

###############################################################################
# **Deep copy**
d = a.copy()
d is a

###############################################################################
d.base is a
