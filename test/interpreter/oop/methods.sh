# lambda test
# --output:start
# symbol a not found
# 15
# sum: 6
# base: 3
# --output:end

class TestBase {
  var x = 5

  func __init__(a) {
    this.c = a
  }

  func GetA() {
    return this.c
  }
}

class Test(TestBase) {
  func __init__(a, b) {
    TestBase.__init__(this, a+2)
    this.a = a
    this.b = b
  }

  func Sum() {
    return this.a + this.b
  }

  func PrintSum() {
    print("sum: ", this.Sum())
  }

  func PrintBase() {
    print("base: ", this.GetA())
  }
}

a = 4

try {
  Test.a = 15
  print(Test.a)
} catch Exception as e {
  print(e)
}

Test.x = 15
print(Test.x)

t = Test(1, 5)
t.PrintSum()
t.PrintBase()
