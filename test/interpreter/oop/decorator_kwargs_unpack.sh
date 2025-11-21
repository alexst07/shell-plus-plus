# Test map unpacking with decorators - the power of ** unpacking!
# --output:start
# === Test 1: Perfect Argument Forwarding ===
# [DEBUG] Calling function with 2 args and 1 kwargs
# Result: 15
# [DEBUG] Function returned
# === Test 2: Caching Decorator ===
# Computing: x=5, y=10
# First call: 15
# [CACHE HIT] for key: 5-10
# Second call: 15
# === Test 3: Validation Decorator ===
# [VALIDATE] Checking args and kwargs
# [VALIDATE] All validations passed
# Processing value: 50 with mode: strict
# === Test 4: Authentication Decorator ===
# [AUTH] Checking credentials from kwargs
# [AUTH] User admin authenticated
# Admin action executed
# === Test 5: Timing Decorator ===
# [TIMER] Starting compute
# Computing sum: 55
# [TIMER] Completed
# === Test 6: Method Decorator ===
# [LOG] Method called on Processor instance
# Processing: important data
# [LOG] Method completed
# --output:end

# Test 1: Perfect argument forwarding with **
func with_logging(level="INFO", config**) {
  return func(f) {
    return func(args..., kwargs**) {
      print("[", level, "] Calling function with ", len(args), " args and ", len(kwargs), " kwargs")
      
      result = f(...args, **kwargs)
      
      print("[", level, "] Function returned")
      return result
    }
  }
}

print("=== Test 1: Perfect Argument Forwarding ===")
@with_logging(level="DEBUG")
func add(x, y) {
  print("Result: ", x + y)
  return x + y
}

add(5, 10)

# Test 2: Caching decorator with kwargs
func memoize(opts**) {
  cache = {}
  
  return func(f) {
    return func(args..., kwargs**) {
      # Create cache key from args and kwargs
      cache_key = str(args[0]) + "-" + str(kwargs["y"]) if kwargs.exists("y") else "default"
      
      if cache.exists(cache_key) {
        print("[CACHE HIT] for key: ", cache_key)
        return cache[cache_key]
      }
      
      print("Computing: x=", args[0], ", y=", kwargs["y"] if kwargs.exists("y") else "none")
      result = f(...args, **kwargs)
      cache[cache_key] = result
      return result
    }
  }
}

print("=== Test 2: Caching Decorator ===")
@memoize()
func add_with_kwargs(x, y=0) {
  return x + y
}

params = {"y": 10}
result1 = add_with_kwargs(5, **params)
print("First call: ", result1)
result2 = add_with_kwargs(5, **params)
print("Second call: ", result2)

# Test 3: Validation decorator with kwargs forwarding
func validate(rules..., opts**) {
  return func(f) {
    return func(args..., kwargs**) {
      print("[VALIDATE] Checking args and kwargs")
      
      # Perform validation
      if len(args) > 0 and args[0] < 0 {
        throw Exception("Invalid value")
      }
      
      print("[VALIDATE] All validations passed")
      return f(...args, **kwargs)
    }
  }
}

print("=== Test 3: Validation Decorator ===")
@validate("positive")
func process(value, mode="normal") {
  print("Processing value: ", value, " with mode: ", mode)
}

process(50, **{"mode": "strict"})

# Test 4: Authentication decorator
func require_auth(role="user", opts**) {
  return func(f) {
    return func(args..., kwargs**) {
      print("[AUTH] Checking credentials from kwargs")
      
      user = kwargs["user"] if kwargs.exists("user") else "anonymous"
      
      if user == "anonymous" {
        throw Exception("Authentication required")
      }
      
      print("[AUTH] User ", user, " authenticated")
      
      # Remove auth params and forward rest
      clean_kwargs = {}
      for key, value in kwargs {
        if key != "user" and key != "role" {
          clean_kwargs[key] = value
        }
      }
      
      return f(...args, **clean_kwargs)
    }
  }
}

print("=== Test 4: Authentication Decorator ===")
@require_auth(role="admin")
func admin_action() {
  print("Admin action executed")
}

auth_params = {"user": "admin", "role": "admin"}
admin_action(**auth_params)

# Test 5: Timing decorator
func timer(config**) {
  return func(f) {
    return func(args..., kwargs**) {
      verbose = config.exists("verbose") and config["verbose"]
      
      if verbose {
        print("[TIMER] Starting ", f.__name__ if "name" in f else "function")
      }
      
      result = f(...args, **kwargs)
      
      if verbose {
        print("[TIMER] Completed")
      }
      
      return result
    }
  }
}

print("=== Test 5: Timing Decorator ===")
@timer(verbose=true)
func compute(n) {
  total = 0
  for i in range(n + 1) {
    total += i
  }
  print("Computing sum: ", total)
  return total
}

compute(10)

# Test 6: Method decorator with kwargs forwarding
func log_method(config**) {
  return func(f) {
    return func(self, args..., kwargs**) {
      print("[LOG] Method called on ", typeof(self), " instance")
      
      result = f(self, ...args, **kwargs)
      
      print("[LOG] Method completed")
      return result
    }
  }
}

print("=== Test 6: Method Decorator ===")
class Processor {
  func __init__() {
    this.name = "Processor"
  }
  
  @log_method(level="INFO")
  func process(data, priority="normal") {
    print("Processing: ", data)
  }
}

proc = Processor()
proc.process(**{"data": "important data", "priority": "high"})

