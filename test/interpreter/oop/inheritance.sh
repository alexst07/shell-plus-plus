class Base {
  func __init__(a) {
    print("__init__ type this: ", type(a))
    this.a = a
    print("__init__ type this: ", type(this.a))
  }

  func printA() {
    dump_symbol_table(this)
    print("type this: ", type(this.a))
    print("str -> value a: ", this.a)
  }

  func __str__() {
    return "value :a" + string(this.a)
  }

  func __add__(x) {
    print("__add__ type this: ", type(this.a))
    y = this.a + x
    print("__add__ type this: ", type(y))
    return Base(y)
  }
}

class Derived(Base) {
  func __init__(b) {
    Base.__init__(this, b + 5)
    this.b = b
  }

  func printB() {
    print("value a: ", this.a)
  }
}

q = Derived(15)
q.printA()
print(string(q))
a = q + 9
a.printA()
