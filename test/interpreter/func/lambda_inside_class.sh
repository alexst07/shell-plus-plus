# test the case where a lambda is returned by some method inside class
#
# BUG: It test is the bug that ocorred when a lambda was returned by some
# some method on the class, and when this lambda function was called
# the caller, tried to pass object of class like this parameter for the
# lambda function
#
# --output:start
# test
# >> other
# --output:end

class Test {
  func __init__(b) {
    this.b = b
    this.x = func(a) {
      print(this.b)
      print(">> ", a)
    }
  }

  func get_x() {
    return this.x
  }
}

t = Test("test")
a = t.get_x()("other")
