# arithmetic with integers variables
ia = 4 + 3 - 2
print("the value of ia is ", ia)

x1 = 74
x2 = 48
x3 = 3
x4 = 6
x5 = 8
x = (x1 + (x5 - x2)*x3)/x4
print("value of x is ", x)

# shift right
sr1 = 7
sr2 = 2
sr = sr1 << sr2
print("value of sr is ", sr)

# shift left
sl1 = 28
sl2 = 2
sl = sr1 >> sr2
print("value of sl is ", sl)

# arithmetic with float variables
f1 = 4.48
f2 = 74.6
f3 = 42.69
f4 = -23,74
f = (f1*f3 + f4)/f2
print("value of f is ", f)

print("value of f + x is ", f + x)
print("value of f - x is ", f - x)
print("value of f * x is ", f * x)
print("value of x / f is ", x / f)
