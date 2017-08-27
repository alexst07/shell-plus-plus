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

print("s" is string)
print(8 is int)
print(15 is bool)

class Foo {}
print(Foo() is Foo)
f = Foo()
print(f is Foo)

interface ITest {}
class Test: ITest {}

t = Test()
print(t is ITest)
print(t is Test)
print(t is object)
print(t is Foo)
