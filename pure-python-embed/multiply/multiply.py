import numpy

def multiply(a,b):
    print("Will compute", a, "times", b)
    c = 0
    for i in range(0, a):
        c = c + b
    return c

def substring(s, start, end):
    # work with deps
    arr = numpy.arange(12).reshape(4, 3)
    print (arr)
    return s[start:end]
