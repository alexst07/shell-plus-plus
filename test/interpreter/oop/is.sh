# simple test for is
# --output:start
# true
# true
# false
# true
# true
# false
# true
# false
# false
# --output:end

print(type("s") is string)
print(type(8) is int)
print(type(15) is bool)

class Foo {}
print(type(Foo()) is Foo)
f = Foo()
print(type(f) is Foo)

interface ITest {}
class Test: ITest {}

t = Test()
print(type(t) is ITest)
print(type(t) is Test)
print(type(t) is object)
print(type(t) is Foo)
