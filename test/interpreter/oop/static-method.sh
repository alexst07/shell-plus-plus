# interfaces tests
# --output:start
# static func
# a: 15
# --output:end

class Test {
  func __init__(a) {
    this.a = a
  }

  func ValA() {
    print("a: ", this.a)
  }

  static func Print() {
    print("static func")
  }
}

Test.Print()

t = Test(15)
t.ValA()
