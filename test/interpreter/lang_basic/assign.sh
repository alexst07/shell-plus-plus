# if else test
# --output:start
# 10
# 15
# --output:end

x = 15
assert(x == 15)
x = "test"
assert(x == "test")

v = [1, 2, 3]
assert(v == [1, 2, 3])
v[0] = 55
assert(v == [55, 2, 3])
v[1] = [10, 11]
assert(v == [55, [10, 11], 3])
v[1][0] = 50
assert(v == [55, [50, 11], 3])

a, b = 4, 5
assert(a == 4 && b == 5)
c = 4 ,5
assert(c[0] == 4)
assert(c[1] == 5)

m = {"test1": "value1", "test2":"value2"}
assert(m["test1"] == "value1")

m["test3"] = 10
assert(m["test3"] == 10)

func t1() {
  return 10, 15
}

a = t1()
assert(a[0] == 10)
assert(a[1] == 15)
a, b = t1()
assert(a == 10)
assert(b == 15)
for i in [a,b] {
  print(i)
}
