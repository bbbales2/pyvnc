import cppimport.import_hook
import ptest
import numpy
import matplotlib.pyplot as plt

h = ptest.Helper(":2")

print(numpy.array(h.getScreen()).dtype)
print h.getScreenDims()

while 1:
    h.handleMessage()
    v = numpy.array(h.getScreen()).reshape(h.getScreenDims())
    plt.imshow(v)
    plt.show()
    #print v.max()
    #print v.min()
