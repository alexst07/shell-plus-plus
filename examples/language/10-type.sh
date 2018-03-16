#
# get type of a variable
#

s = "test"
print("type(s) -> ", type(s))

i = 4
print("type(i) -> ", type(i))

f = 5.9
print("type(f) -> ", type(f))

a = [1, "str", 1.6]
print("type(a) -> ", type(a))

m = {"key": 7}
print("type(m) -> ", type(m))

n = null
print("type(n) -> ", type(n))

b = true
print("type(b) -> ", type(b))

t = type(5)
print("type(t) -> ", type(t))

#
# convert from one type to another
#

# convert string to integer
s = "4"
i = int(i)
print("i + 3 -> ", i + 3)

# convert integer to string
i = 7
s = string(i)
print("my string: " + s)

# convert from string to real
s = "4.3"
f = real(f)
print("f + 3 -> ", f + 3)

# convert from real to string
f = 7.32
s = string(f)
print("my string: " + s)
