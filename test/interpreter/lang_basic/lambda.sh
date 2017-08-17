# simple lambda test
# --output:start
# 16
# 64
# 1
# 2
# 3
# --output:end

l = lambda x: x*x
print(l(4))

v = [lambda x: print(x), lambda x: x*x*x]

print(v[1](4))
t = [1,2,3]
t.for_each(lambda x:print(x))
