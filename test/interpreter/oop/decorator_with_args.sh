# Advanced decorator syntax tests - functions with arguments
# --output:start
# Decorator called with: value1
# Function decorated: [function]
# Function called with: 10 20
# Result: 30
# Decorator called with: prefix
# Function decorated: [function]
# Calling greet with: Alice Bob
# Hello Alice and Bob
# Decorator called with: 3
# Function decorated: [function]
# Attempt 1
# multiply called with 5 and 7
# Result: 35
# Decorator called with: debug
# Function decorated: [function]
# compute called with a=100, b=50, c=25
# Result from compute: 175
# --output:end

# Test 1: Decorator factory - decorator that takes arguments
func make_decorator(decorator_arg) {
  print("Decorator called with: ", decorator_arg)
  
  func actual_decorator(f) {
    print("Function decorated: ", f)
    return f
  }
  
  return actual_decorator
}

@make_decorator("value1")
func add(x, y) {
  print("Function called with: ", x, " ", y)
  return x + y
}

result = add(10, 20)
print("Result: ", result)

# Test 2: Decorator factory with string argument
@make_decorator("prefix")
func greet(name1, name2) {
  print("Hello ", name1, " and ", name2)
}

print("Calling greet with: Alice Bob")
greet("Alice", "Bob")

# Test 3: Decorator with numeric argument
@make_decorator(3)
func multiply(a, b) {
  print("multiply called with ", a, " and ", b)
  return a * b
}

print("Attempt 1")
result2 = multiply(5, 7)
print("Result: ", result2)

# Test 4: Function with multiple arguments
@make_decorator("debug")
func compute(a, b, c) {
  print("compute called with a=", a, ", b=", b, ", c=", c)
  return a + b + c
}

result3 = compute(100, 50, 25)
print("Result from compute: ", result3)

