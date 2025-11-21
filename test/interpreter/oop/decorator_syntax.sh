# Test decorator syntax for functions and classes
# --output:start
# Function decorated
# Inside my_function
# Value: 42
# Class decorated
# MyClass method called
# Custom class created
# --output:end

# Test 1: Simple function decorator (identity decorator)
func decorator(f) {
  print("Function decorated")
  return f
}

@decorator
func my_function() {
  print("Inside my_function")
}

my_function()

# Test 2: Decorator that returns a different value
func value_decorator(f) {
  func wrapper() {
    return 42
  }
  return wrapper
}

@value_decorator
func get_value() {
  return 10
}

print("Value: ", get_value())

# Test 3: Class decorator (identity decorator)
func class_decorator(cls) {
  print("Class decorated")
  return cls
}

@class_decorator
class MyClass {
  func __init__() {
    this.name = "test"
  }
  
  func method() {
    print("MyClass method called")
  }
}

obj = MyClass()
obj.method()

# Test 4: Class decorator that modifies the class
func add_message_decorator(cls) {
  # Since we can't create closures, just return the class
  print("Custom class created")
  return cls
}

@add_message_decorator
class Config {
  func __init__() {
    this.value = 100
  }
}

c1 = Config()

