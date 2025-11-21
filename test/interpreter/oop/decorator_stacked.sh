# Test stacked decorators (multiple decorators on one function)
# --output:start
# === Test 1: Two Decorators on Function ===
# [OUTER] Decorating function
# [INNER] Decorating function
# [OUTER] Calling function
# [INNER] Calling function
# Original function called with: 5, 10
# Result: 15
# === Test 2: Three Decorators ===
# [FIRST] Decorating
# [SECOND] Decorating
# [THIRD] Decorating
# [FIRST] Before call
# [SECOND] Before call
# [THIRD] Before call
# multiply: 6
# [THIRD] After call
# [SECOND] After call
# [FIRST] After call
# === Test 3: Decorators with Arguments ===
# [LEVEL1] Config: {"priority": "high"}
# [LEVEL2] Config: {"mode": "strict"}
# [LEVEL1] Executing
# [LEVEL2] Executing
# Processing data: test
# === Test 4: Method Decorators (Stacked) ===
# [AUTH] Checking permissions
# [LOG] Method called
# Processing: secure data
# [LOG] Method completed
# [AUTH] Access granted
# --output:end

# Test 1: Two decorators on a function
func outer_decorator(f) {
  print("[OUTER] Decorating function")
  
  return func(args...) {
    print("[OUTER] Calling function")
    result = f(...args)
    return result
  }
}

func inner_decorator(f) {
  print("[INNER] Decorating function")
  
  return func(args...) {
    print("[INNER] Calling function")
    result = f(...args)
    return result
  }
}

print("=== Test 1: Two Decorators on Function ===")
@outer_decorator
@inner_decorator
func add(x, y) {
  print("Original function called with: ", x, ", ", y)
  return x + y
}

result = add(5, 10)
print("Result: ", result)

# Test 2: Three decorators
func first(f) {
  print("[FIRST] Decorating")
  return func(args...) {
    print("[FIRST] Before call")
    result = f(...args)
    print("[FIRST] After call")
    return result
  }
}

func second(f) {
  print("[SECOND] Decorating")
  return func(args...) {
    print("[SECOND] Before call")
    result = f(...args)
    print("[SECOND] After call")
    return result
  }
}

func third(f) {
  print("[THIRD] Decorating")
  return func(args...) {
    print("[THIRD] Before call")
    result = f(...args)
    print("[THIRD] After call")
    return result
  }
}

print("=== Test 2: Three Decorators ===")
@first
@second
@third
func multiply(a, b) {
  result = a * b
  print("multiply: ", result)
  return result
}

multiply(2, 3)

# Test 3: Stacked decorators with arguments
func make_level1(config**) {
  print("[LEVEL1] Config: ", config)
  
  return func(f) {
    return func(args...) {
      print("[LEVEL1] Executing")
      return f(...args)
    }
  }
}

func make_level2(config**) {
  print("[LEVEL2] Config: ", config)
  
  return func(f) {
    return func(args...) {
      print("[LEVEL2] Executing")
      return f(...args)
    }
  }
}

print("=== Test 3: Decorators with Arguments ===")
@make_level1(priority="high")
@make_level2(mode="strict")
func process_data(data) {
  print("Processing data: ", data)
}

process_data("test")

# Test 4: Stacked decorators on methods
func auth_decorator(f) {
  return func(self, args...) {
    print("[AUTH] Checking permissions")
    result = f(self, ...args)
    print("[AUTH] Access granted")
    return result
  }
}

func log_decorator(f) {
  return func(self, args...) {
    print("[LOG] Method called")
    result = f(self, ...args)
    print("[LOG] Method completed")
    return result
  }
}

print("=== Test 4: Method Decorators (Stacked) ===")
class SecureProcessor {
  func __init__() {
    this.name = "SecureProcessor"
  }
  
  @auth_decorator
  @log_decorator
  func process(data) {
    print("Processing: ", data)
  }
}

proc = SecureProcessor()
proc.process("secure data")

