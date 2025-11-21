# Test decorator syntax for methods inside classes
# --output:start
# Method decorated
# Static method decorated
# MyClass method called
# Static method called
# --output:end

# Test method decorators inside classes
func method_decorator(f) {
  print("Method decorated")
  return f
}

func static_decorator(f) {
  print("Static method decorated")
  return f
}

class MyClass {
  func __init__() {
    this.name = "test"
  }
  
  @method_decorator
  func method() {
    print("MyClass method called")
  }
  
  @static_decorator
  static func static_method() {
    print("Static method called")
  }
}

obj = MyClass()
obj.method()
MyClass.static_method()

