# simple class with same name of attributes
# --output:start
# symbol a not found
# {(b, 5), (a, 1)}
# {(b, 5), (a, 1)}
# [function]
# [function]
# [function]
# [this, a, b, c]
# symbol a not found
# [a, b]
# --output:end

class Base {
  func Okkkk(a, b, c...) {}
}

class TestBase(Base) {
  func __init__(c) {
    this.a = c
  }

  func GetFuncs() {
    return get_attr_type(this)
  }
}

class Test(TestBase) {
  func __init__(a, b) {
    this.a = a
    this.b = b
  }

  func GetAttrObj() {
    return get_attr_obj(this)
  }

  func GetAttrType(b = 3) {
    return get_attr_type(this)
  }

  func GetA() {
    return this.a
  }

  func GetB() {
    return this.b
  }
}

a = 5
t = Test(1, 5)

try {
  b = []
  print(b.a)
} catch Exception as e {
  print(e)
}

print(get_attr_obj(t))
print(t.GetAttrObj())
print(get_attr_type(t)["GetA"])
print(t.GetAttrType()["GetFuncs"])
print(t.GetFuncs()["Okkkk"])
print(get_attr_type(t)["Okkkk"].__params__)

try {
  print(get_attr_type(t)["Okkkk"].a)
} catch Exception as e {
  print(e)
}

func fTest(a, b) {}

print(fTest.__params__)
