# tuple test
# --output:start
# ()
# (1, test)
# (4, (1, 2))
# (4, (90, 2))
# {(n, ()), (as, (4, 2)), (other, ([function], 5))}
# ((1, 2, 3, 10), 5)
# --output:end

a = ()
print(a)

b, c = (1, "test")
print((b, c))

t1, t2 = 4, (1, 2)
print((t1, t2))
t2[0] = 90
print((t1, t2))

d = {"as": (4, 2),
     "other": (lambda x: x, 5),
     "n": ()}
print(d)

a = (1, 2, 3)
b = (...a, 10), 5
print(b)
