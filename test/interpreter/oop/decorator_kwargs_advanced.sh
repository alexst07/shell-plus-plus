# Test decorators with kwargs - executing functions with configuration
# --output:start
# === Test 1: Logging Decorator with kwargs ===
# [LOG] Calling function with 2 arguments
# [INFO] Executing function
# Result: 15
# === Test 2: Cache Decorator ===
# Computing expensive operation for: 5
# First call result: 25
# [CACHE HIT] Returning cached result
# Second call result: 25
# === Test 3: Retry Decorator ===
# Attempt 1 failed
# Attempt 2 failed
# Attempt 3 succeeded
# Result: success
# === Test 4: Timing Decorator ===
# [TIMER] Function started
# Sum calculation: 55
# [TIMER] Function completed
# === Test 5: Validation Decorator ===
# [VALIDATE] Checking min=0, max=100
# Value 50 is valid
# Result: 50
# === Test 6: Transform Decorator ===
# Original arg 1: hello
# Original arg 2: world
# Transformed: HELLO WORLD
# === Test 7: Method Decorator with kwargs ===
# [METHOD LOG] Calling method with verbose: true
# Processing: test data
# [METHOD LOG] Method completed
# --output:end

# Test 1: Logging decorator with configuration via kwargs
func with_logging(options**) {
  return func(f) {
    return func(args...) {
      print("[LOG] Calling function with ", len(args), " arguments")
      
      if options.exists("level") {
        print("[", options["level"], "] Executing function")
      }
      
      result = f(...args)
      return result
    }
  }
}

print("=== Test 1: Logging Decorator with kwargs ===")
@with_logging(level="INFO")
func add(x, y) {
  return x + y
}

result1 = add(5, 10)
print("Result: ", result1)

# Test 2: Cache decorator that stores results
func memoize(opts**) {
  cache = {}
  
  return func(f) {
    return func(args...) {
      # Create cache key from args (simplified)
      cache_key = str(args[0]) if len(args) > 0 else "default"
      
      if cache.exists(cache_key) {
        print("[CACHE HIT] Returning cached result")
        return cache[cache_key]
      }
      
      print("Computing expensive operation for: ", args[0])
      result = f(...args)
      cache[cache_key] = result
      return result
    }
  }
}

print("=== Test 2: Cache Decorator ===")
@memoize()
func expensive_computation(n) {
  return n * n
}

result2a = expensive_computation(5)
print("First call result: ", result2a)
result2b = expensive_computation(5)
print("Second call result: ", result2b)

# Test 3: Retry decorator with configurable attempts
func retry(attempts=3, opts**) {
  return func(f) {
    return func(args...) {
      counter = 0
      while counter < attempts {
        counter += 1
        
        try {
          return f(...args)
        } catch e {
          if counter < attempts {
            print("Attempt ", counter, " failed")
          } else {
            throw e
          }
        }
      }
    }
  }
}

print("=== Test 3: Retry Decorator ===")
global_counter = 0

@retry(3)
func flaky_function() {
  global global_counter
  global_counter += 1
  
  if global_counter < 3 {
    # Simulate failure - throw error
    throw Exception("Simulated failure")
  }
  
  print("Attempt ", global_counter, " succeeded")
  return "success"
}

result3 = flaky_function()
print("Result: ", result3)

# Test 4: Timing decorator
func timer(config**) {
  return func(f) {
    return func(args...) {
      verbose = config.exists("verbose") and config["verbose"]
      
      if verbose {
        print("[TIMER] Function started")
      }
      
      result = f(...args)
      
      if verbose {
        print("[TIMER] Function completed")
      }
      
      return result
    }
  }
}

print("=== Test 4: Timing Decorator ===")
@timer(verbose=true)
func compute_sum(n) {
  total = 0
  for i in range(n + 1) {
    total += i
  }
  print("Sum calculation: ", total)
  return total
}

compute_sum(10)

# Test 5: Validation decorator
func validate(min=0, max=100, opts**) {
  return func(f) {
    return func(value, args...) {
      print("[VALIDATE] Checking min=", min, ", max=", max)
      
      if value < min or value > max {
        throw Exception("Value out of range")
      }
      
      print("Value ", value, " is valid")
      return f(value, ...args)
    }
  }
}

print("=== Test 5: Validation Decorator ===")
@validate(min=0, max=100)
func process_value(val) {
  return val
}

result5 = process_value(50)
print("Result: ", result5)

# Test 6: Transform arguments decorator
func transform(mode="upper", opts**) {
  return func(f) {
    return func(args...) {
      # Transform string arguments
      new_args = []
      
      for i in range(len(args)) {
        arg = args[i]
        print("Original arg ", i + 1, ": ", arg)
        
        if typeof(arg) == "string" {
          if mode == "upper" {
            new_args.append(arg.upper())
          } else {
            new_args.append(arg)
          }
        } else {
          new_args.append(arg)
        }
      }
      
      return f(...new_args)
    }
  }
}

print("=== Test 6: Transform Decorator ===")
@transform(mode="upper")
func echo(args...) {
  print("Transformed: ", ...args)
}

echo("hello", "world")

# Test 7: Decorator for class methods with kwargs
func method_logger(config**) {
  return func(f) {
    return func(self, args...) {
      verbose = config.exists("verbose") and config["verbose"]
      
      if verbose {
        print("[METHOD LOG] Calling method with verbose: ", config["verbose"])
      }
      
      result = f(self, ...args)
      
      if verbose {
        print("[METHOD LOG] Method completed")
      }
      
      return result
    }
  }
}

print("=== Test 7: Method Decorator with kwargs ===")
class Processor {
  func __init__() {
    this.data = "test data"
  }
  
  @method_logger(verbose=true)
  func process() {
    print("Processing: ", this.data)
  }
}

proc = Processor()
proc.process()
