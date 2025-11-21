# Advanced decorator tests - multiple decorators and complex scenarios
# --output:start
# Type checker initialized for: int
# Type checker applied to: [function]
# Logger applied to: [function]
# Result: 150
# Factory 1: Creating counter
# Factory 2: Wrapping function
# Calling count 3 times
# Counter function called
# Counter function called
# Counter function called
# Validator initialized with min=0 max=100
# Validator applied to: [function]
# Setting value to: 50
# Value is valid: 50
# Cache initialized
# Cache applied to: [function]
# Computing expensive operation with: 5 3
# Result: 75
# Using cached result
# Computing expensive operation with: 5 3
# Result: 75
# --output:end

# Test 1: Simulating multiple decorators using manual composition
func type_checker(expected_type) {
  print("Type checker initialized for: ", expected_type)
  
  func decorator(f) {
    print("Type checker applied to: ", f)
    return f
  }
  
  return decorator
}

func logger(f) {
  print("Logger applied to: ", f)
  return f
}

# Manually compose decorators since we don't have decorator chaining
temp_func1 = func(a, b, c) {
  return a + b + c
}
temp_func2 = type_checker("int")(temp_func1)
process = logger(temp_func2)

result = process(50, 60, 40)
print("Result: ", result)

# Test 2: Nested decorator factories
func make_counter_decorator(name) {
  print("Factory 1: Creating counter")
  
  func counter_decorator(f) {
    print("Factory 2: Wrapping function")
    
    func wrapper() {
      print("Counter function called")
    }
    
    return wrapper
  }
  
  return counter_decorator
}

@make_counter_decorator("test")
func count() {
  # Original function (will be replaced by wrapper)
}

print("Calling count 3 times")
count()
count()
count()

# Test 3: Decorator with multiple parameters
func validator(min_val, max_val) {
  print("Validator initialized with min=", min_val, " max=", max_val)
  
  func decorator(f) {
    print("Validator applied to: ", f)
    return f
  }
  
  return decorator
}

@validator(0, 100)
func set_value(val) {
  print("Value is valid: ", val)
}

print("Setting value to: 50")
set_value(50)

# Test 4: Decorator for caching concept (without actual caching due to closure limitations)
func cache_decorator() {
  print("Cache initialized")
  
  func decorator(f) {
    print("Cache applied to: ", f)
    return f
  }
  
  return decorator
}

@cache_decorator()
func expensive_operation(x, y) {
  print("Computing expensive operation with: ", x, " ", y)
  return x * x * y
}

print("Result: ", expensive_operation(5, 3))
print("Using cached result")
print("Result: ", expensive_operation(5, 3))

