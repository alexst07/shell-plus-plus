# simple test for instanceof
# --output:start
# true
# false
# true
# true
# false
# false
# false
# true
# true
# true
# true
# false
# --output:end

interface Iface1 {}
interface Iface2 {}

class Test1: Iface1 {}
class Test2(Test1): Iface2 {}

class Foo {}

f = Foo()
t1 = Test1()
t2 = Test2()

print(f instanceof Foo)
print(f instanceof Iface1)
print(t1 instanceof Iface1)
print(t1 instanceof Test1)
print(t1 instanceof Test2)
print(t1 instanceof Iface2)
print(t1 instanceof Foo)
print(t2 instanceof Iface1)
print(t2 instanceof Test1)
print(t2 instanceof Test2)
print(t2 instanceof Iface2)
print(t2 instanceof Foo)
