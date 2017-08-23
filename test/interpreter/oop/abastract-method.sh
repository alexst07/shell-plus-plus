# simple class with same name of attributes
# --output:start
# val a: 4 val b: 15
# --output:end

interface IFoo {
  func func_foo()
}

abstract class AbstractTest: IFoo {
  func __init__(a) {
    this.a = a
  }

  abstract func Fother(a)

  func func_foo() {}

  func ValA() {
    print("val a: ", this.a)
  }
}

class Test(AbstractTest) {
  func __init__(b) {
    AbstractTest.__init__(this, 4)
    this.b = b
  }

  func Val() {
    print("val a: ", this.a, " val b: ", this.b)
  }

  func Fother(a) {
    print("Fother")
  }
}

t = Test(15)
t.Val()
