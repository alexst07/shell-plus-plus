# simple class with same name of attributes
# --output:start
# 5
# 3
# x: 10
# --output:end

class test {
  func __init__(x) {
    this.a = x
  }

  func get() {
    return this.a
  }

  func obj() {
    return this
  }
}

class test2 {
  func __init__(x) {
    this.a = x
  }

  func get() {
    return this.a
  }

  func obj() {
    return this
  }
}

x = 10

t1 = test(5)
t2 = test2(3)

print(t1.obj().get())
print(t2.obj().get())

print("x: ", x)
