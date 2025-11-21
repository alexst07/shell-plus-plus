# Decorator tests for methods with arguments
# --output:start
# Method validator initialized for: process_data
# Method decorated
# Processing: item1 item2 item3
# Items processed successfully
# Static validator initialized
# Static method decorated
# Static method called with: 100 200
# Sum: 300
# Method logger initialized
# Method decorated
# Calculator: add operation
# Adding 15 and 25
# Result from calculator: 40
# Initializer decorator applied
# Instance created with: TestValue
# Getting value: TestValue
# --output:end

# Test 1: Method decorator factory with arguments
func validate_method(method_name) {
  print("Method validator initialized for: ", method_name)
  
  func decorator(f) {
    print("Method decorated")
    return f
  }
  
  return decorator
}

class DataProcessor {
  func __init__() {
    this.name = "Processor"
  }
  
  @validate_method("process_data")
  func process(item1, item2, item3) {
    print("Processing: ", item1, " ", item2, " ", item3)
    print("Items processed successfully")
  }
}

processor = DataProcessor()
processor.process("item1", "item2", "item3")

# Test 2: Static method decorator with arguments
func validate_static(f) {
  print("Static validator initialized")
  
  func decorator(fn) {
    print("Static method decorated")
    return fn
  }
  
  return decorator(f)
}

class MathUtils {
  @validate_static
  static func add_numbers(a, b) {
    print("Static method called with: ", a, " ", b)
    return a + b
  }
}

result = MathUtils.add_numbers(100, 200)
print("Sum: ", result)

# Test 3: Multiple parameters on decorated methods
func log_method() {
  print("Method logger initialized")
  
  func decorator(f) {
    print("Method decorated")
    return f
  }
  
  return decorator
}

class Calculator {
  func __init__(name) {
    this.calc_name = name
  }
  
  @log_method()
  func add(x, y) {
    print("Calculator: add operation")
    print("Adding ", x, " and ", y)
    return x + y
  }
}

calc = Calculator("MyCalc")
result2 = calc.add(15, 25)
print("Result from calculator: ", result2)

# Test 4: Decorator affecting instance creation
func init_decorator(cls) {
  print("Initializer decorator applied")
  return cls
}

@init_decorator
class TestClass {
  func __init__(value) {
    this.value = value
    print("Instance created with: ", value)
  }
  
  func get_value() {
    print("Getting value: ", this.value)
  }
}

test_obj = TestClass("TestValue")
test_obj.get_value()

