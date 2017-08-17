# simple class with same name of attributes
# --output:start
# hello+world
# 11
# ok
# l
# a>
# --output:end

class test {
  func __init__(x) {
    this.a = x
  }

  func __add__(x) {
    this.a += "+" + x
    return this
  }

  func __sub__(x) {
    this.a += "-" + x
    return this
  }

  func __call__(a, b, c) {
    return "call:(" + a +", " + b + ", " + c + ")"
  }

  func __print__() {
    return this.a
  }

  func __hash__() {
    return 8
  }

  func __len__() {
    return len(this.a)
  }

  func __bool__() {
    return len(this.a) > 10
  }

  func __getitem__(i) {
    return this.a[i]
  }
}

t = test("hello")
t = t+"world"
print(t)
print(len(t))

if t {
  print("ok")
}

print(t[3])

a = test("ok")
if a {
  print("a<")
} else {
  print("a>")
}
