# Test decorators executing and modifying function behavior with kwargs
# --output:start
# === Test 1: Authenticate Decorator ===
# [AUTH] Checking authentication...
# [AUTH] User admin authenticated with role: admin
# Accessing protected resource
# Result: Protected data accessed
# === Test 2: Rate Limiter ===
# [RATE LIMIT] Call 1 allowed
# API called: request 1
# [RATE LIMIT] Call 2 allowed
# API called: request 2
# [RATE LIMIT] Call 3 allowed
# API called: request 3
# === Test 3: Deprecation Warning ===
# [DEPRECATED] Function old_function is deprecated. Use new_function instead
# Old function executing
# === Test 4: Type Checker ===
# [TYPE CHECK] Validating argument types
# [TYPE CHECK] Arg 1: int OK
# [TYPE CHECK] Arg 2: string OK
# Processing: 42 hello
# === Test 5: Event Emitter ===
# [EVENT] before_call triggered for save_data
# Saving data: important info
# [EVENT] after_call triggered for save_data
# Data saved successfully
# === Test 6: Auto-retry with Exponential Backoff ===
# [RETRY] Attempt 1 (delay: 1)
# Operation attempt 1
# [RETRY] Attempt 2 (delay: 2)
# Operation attempt 2
# [RETRY] Success after 2 attempts
# Result: Success on attempt 2
# === Test 7: Permission Checker ===
# [PERMISSION] Checking permissions: admin=true
# [PERMISSION] Access granted
# Admin function executed
# --output:end

# Test 1: Authentication decorator with role checking
func require_auth(role="user", opts**) {
  return func(f) {
      return func(args...) {
      print("[AUTH] Checking authentication...")
      
      # Simulate authentication check
      user = kwargs["user"] if kwargs.exists("user") else "anonymous"
      user_role = kwargs["role"] if kwargs.exists("role") else "guest"
      
      if user == "anonymous" {
        throw Exception("Authentication required")
      }
      
      if role == "admin" and user_role != "admin" {
        throw Exception("Admin access required")
      }
      
      print("[AUTH] User ", user, " authenticated with role: ", user_role)
      
      # Remove auth params before calling function
      clean_kwargs = {}
      for key, value in kwargs {
        if key != "user" and key != "role" {
          clean_kwargs[key] = value
        }
      }
      
      return f(...args)
    }
  }
}

print("=== Test 1: Authenticate Decorator ===")
@require_auth(role="admin")
func protected_resource() {
  print("Accessing protected resource")
  return "Protected data accessed"
}

result1 = protected_resource(user="admin", role="admin")
print("Result: ", result1)

# Test 2: Rate limiter decorator
func rate_limit(max_calls=3, opts**) {
  call_count = 0
  
  return func(f) {
      return func(args...) {
      global call_count
      call_count += 1
      
      if call_count > max_calls {
        throw Exception("Rate limit exceeded")
      }
      
      print("[RATE LIMIT] Call ", call_count, " allowed")
      return f(...args)
    }
  }
}

print("=== Test 2: Rate Limiter ===")
@rate_limit(max_calls=3)
func api_call(request_id) {
  print("API called: ", request_id)
  return "Success"
}

api_call("request 1")
api_call("request 2")
api_call("request 3")

# Test 3: Deprecation warning decorator
func deprecated(alternative="", opts**) {
  return func(f) {
      return func(args...) {
      msg = "[DEPRECATED] Function is deprecated"
      if alternative != "" {
        msg = "[DEPRECATED] Function old_function is deprecated. Use " + alternative + " instead"
      }
      print(msg)
      
      return f(...args)
    }
  }
}

print("=== Test 3: Deprecation Warning ===")
@deprecated(alternative="new_function")
func old_function() {
  print("Old function executing")
}

old_function()

# Test 4: Type checker decorator
func type_check(types..., opts**) {
  return func(f) {
      return func(args...) {
      print("[TYPE CHECK] Validating argument types")
      
      # Check types match
      for i in range(min(len(types), len(args))) {
        expected = types[i]
        actual = typeof(args[i])
        
        if expected == "int" and actual == "int" {
          print("[TYPE CHECK] Arg ", i + 1, ": int OK")
        } elif expected == "string" and actual == "string" {
          print("[TYPE CHECK] Arg ", i + 1, ": string OK")
        }
      }
      
      return f(...args)
    }
  }
}

print("=== Test 4: Type Checker ===")
@type_check("int", "string")
func process_data(num, text) {
  print("Processing: ", num, " ", text)
}

process_data(42, "hello")

# Test 5: Event emitter decorator
func emit_events(events..., opts**) {
  return func(f) {
      return func(args...) {
      # Emit before event
      print("[EVENT] before_call triggered for ", f.__name__ if "name" in f else "function")
      
      result = f(...args, **kwargs)
      
      # Emit after event
      print("[EVENT] after_call triggered for ", f.__name__ if "name" in f else "function")
      
      return result
    }
  }
}

print("=== Test 5: Event Emitter ===")
@emit_events("before_call", "after_call")
func save_data(data) {
  print("Saving data: ", data)
  return "Data saved successfully"
}

result5 = save_data("important info")
print(result5)

# Test 6: Auto-retry with backoff
func retry_with_backoff(max_attempts=3, opts**) {
  return func(f) {
      return func(args...) {
      attempt = 0
      
      while attempt < max_attempts {
        attempt += 1
        print("[RETRY] Attempt ", attempt, " (delay: ", attempt, ")")
        
        try {
          result = f(...args, **kwargs)
          print("[RETRY] Success after ", attempt, " attempts")
          return result
        } catch e {
          if attempt >= max_attempts {
            throw e
          }
          # In real implementation, would add delay here
        }
      }
    }
  }
}

print("=== Test 6: Auto-retry with Exponential Backoff ===")
attempt_counter = 0

@retry_with_backoff(max_attempts=5)
func unreliable_operation() {
  global attempt_counter
  attempt_counter += 1
  
  print("Operation attempt ", attempt_counter)
  
  if attempt_counter < 2 {
    throw Exception("Temporary failure")
  }
  
  return "Success on attempt " + str(attempt_counter)
}

result6 = unreliable_operation()
print("Result: ", result6)

# Test 7: Permission checker for methods
func require_permission(perms..., opts**) {
  return func(f) {
    return func(self, args...) {
      # Check permissions from opts
      has_admin = opts.exists("admin") and opts["admin"]
      
      print("[PERMISSION] Checking permissions: admin=", has_admin)
      
      if not has_admin {
        throw Exception("Insufficient permissions")
      }
      
      print("[PERMISSION] Access granted")
      return f(self, ...args, **kwargs)
    }
  }
}

print("=== Test 7: Permission Checker ===")
class SecureResource {
  func __init__() {
    this.data = "secure"
  }
  
  @require_permission("write", admin=true)
  func admin_function() {
    print("Admin function executed")
  }
}

resource = SecureResource()
resource.admin_function()

