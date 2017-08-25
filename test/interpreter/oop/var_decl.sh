# inner class test
# --output:start
# 8
# oktest: 8
# 8
# [25, 10]
# oktest: [25, 10]
# 8
# --output:end

class Test {
  var x = 8

  func fn() {
    return "ok"
  }

  func Print() {
    print(this.fn(), "test: ", Test.x)
  }
}

t = Test()
print(Test.x)
t.Print()
Test.x = [15, 10]
print(t.x)
Test.x[0] = 25
print(Test.x)
t.Print()
print(t.x)
