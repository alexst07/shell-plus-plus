# if else test
# --output:start
# 1
# test
# true
# --output:end

t = 1, "test", true

assert(type(t) == tuple)
assert(type(array(t)) == array)

assert(t[0] == 1)
assert(t[1] == "test")
assert(t[2] == true)

for i in t {
  print(i)
}
