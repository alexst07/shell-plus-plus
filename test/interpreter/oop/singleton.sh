# simple class with same name of attributes
# --output:start
# value a: 5, value b: 10
# value a: 8, value b: 9
# --output:end

class Singleton {
  var instance_ = null

  func __init__() {
    this.a = 5
    this.b = 10
  }

  func set_values(a, b) {
    this.a = a
    this.b = b
  }

  func print_values() {
    print("value a: ", this.a, ", value b: ", this.b)
  }

  static func instance() {
    if type(Singleton.instance_) == null_t {
      return let Singleton.instance_ = Singleton()
    } else {
      return Singleton.instance_
    }
  }
}

t = Singleton.instance()
t.print_values()
q = Singleton.instance()
q.set_values(8, 9)
t.print_values()
