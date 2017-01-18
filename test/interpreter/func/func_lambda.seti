# lambda test
# --output:start
# (1, 2, (3, alex))
# 1
# 2
# --output:end

f = func(t...) {
  print(t[0])
  return t
}

g = func(a, b, c...) {
  return a, b, c
}

gres = g(1, 2, 3, "alex")
print(gres)
assert(gres[0] == 1, "error")

print(f(1, 2, 3)[1])
