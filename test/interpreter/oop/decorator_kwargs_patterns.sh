# Test practical decorator patterns with kwargs for real-world scenarios
# --output:start
# === Test 1: Database Transaction Decorator ===
# [TRANSACTION] Starting transaction with autocommit=false
# [DB] Inserting user: Alice
# [DB] Inserting user: Bob
# [TRANSACTION] Committing transaction
# Users created: 2
# === Test 2: HTTP Request Decorator ===
# [HTTP] GET request to /api/users
# [HTTP] Headers: {(auth, Bearer token123)}
# Fetching users from API
# [HTTP] Response status: 200
# === Test 3: Circuit Breaker ===
# [CIRCUIT] State: CLOSED
# Service call 1 succeeded
# [CIRCUIT] State: CLOSED
# Service call 2 succeeded
# [CIRCUIT] State: CLOSED
# Service call 3 succeeded
# === Test 4: Schema Validator ===
# [SCHEMA] Validating input data
# [SCHEMA] Field 'name' present: true
# [SCHEMA] Field 'age' present: true
# [SCHEMA] Validation passed
# Creating user: John age: 30
# === Test 5: Metrics Collector ===
# [METRICS] Recording call to calculate
# Calculating: 100
# [METRICS] Execution time recorded
# [METRICS] Function called 1 times
# Result: 200
# === Test 6: Conditional Execution ===
# [CONDITIONAL] Checking condition: enabled=true
# [CONDITIONAL] Condition met, executing function
# Feature enabled - executing
# === Test 7: Batch Processor ===
# [BATCH] Processing batch of 3 items
# Processing item: item1
# Processing item: item2
# Processing item: item3
# [BATCH] Batch completed successfully
# --output:end

# Test 1: Database transaction decorator
func transaction(autocommit=false, opts**) {
  return func(f) {
      return func(args...) {
      print("[TRANSACTION] Starting transaction with autocommit=", autocommit)
      
      try {
        result = f(...args, **kwargs)
        
        if not autocommit {
          print("[TRANSACTION] Committing transaction")
        }
        
        return result
      } catch e {
        print("[TRANSACTION] Rolling back transaction")
        throw e
      }
    }
  }
}

print("=== Test 1: Database Transaction Decorator ===")
@transaction(autocommit=false)
func create_users(users...) {
  for user in users {
    print("[DB] Inserting user: ", user)
  }
  return len(users)
}

count = create_users("Alice", "Bob")
print("Users created: ", count)

# Test 2: HTTP request decorator with headers
func http_request(method="GET", endpoint="/", opts**) {
  return func(f) {
      return func(args...) {
      print("[HTTP] ", method, " request to ", endpoint)
      
      if opts.exists("headers") {
        print("[HTTP] Headers: ", opts["headers"])
      }
      
      result = f(...args, **kwargs)
      
      print("[HTTP] Response status: 200")
      return result
    }
  }
}

print("=== Test 2: HTTP Request Decorator ===")
@http_request(method="GET", endpoint="/api/users", headers={"auth": "Bearer token123"})
func fetch_users() {
  print("Fetching users from API")
  return ["user1", "user2", "user3"]
}

users = fetch_users()

# Test 3: Circuit breaker pattern
func circuit_breaker(failure_threshold=3, opts**) {
  failures = 0
  state = "CLOSED"  # CLOSED, OPEN, HALF_OPEN
  
  return func(f) {
      return func(args...) {
      global failures, state
      
      if state == "OPEN" {
        print("[CIRCUIT] Circuit is OPEN, rejecting call")
        throw Exception("Circuit breaker is OPEN")
      }
      
      print("[CIRCUIT] State: ", state)
      
      try {
        result = f(...args, **kwargs)
        failures = 0
        state = "CLOSED"
        return result
      } catch e {
        failures += 1
        
        if failures >= failure_threshold {
          state = "OPEN"
          print("[CIRCUIT] Too many failures, opening circuit")
        }
        
        throw e
      }
    }
  }
}

print("=== Test 3: Circuit Breaker ===")
call_number = 0

@circuit_breaker(failure_threshold=5)
func unstable_service() {
  global call_number
  call_number += 1
  
  # Simulate successful calls
  print("Service call ", call_number, " succeeded")
  return "success"
}

unstable_service()
unstable_service()
unstable_service()

# Test 4: Schema validator decorator
func validate_schema(required_fields..., opts**) {
  return func(f) {
    return func(data, args..., kwargs**) {
      print("[SCHEMA] Validating input data")
      
      for field in required_fields {
        has_field = data.exists(field)
        print("[SCHEMA] Field '", field, "' present: ", has_field)
        
        if not has_field {
          throw Exception("Missing required field: " + field)
        }
      }
      
      print("[SCHEMA] Validation passed")
      return f(data, ...args, **kwargs)
    }
  }
}

print("=== Test 4: Schema Validator ===")
@validate_schema("name", "age")
func create_user(user_data) {
  print("Creating user: ", user_data["name"], " age: ", user_data["age"])
  return "User created"
}

user_data = {"name": "John", "age": 30, "email": "john@example.com"}
create_user(user_data)

# Test 5: Metrics collector
func collect_metrics(metric_name="", opts**) {
  call_count = 0
  
  return func(f) {
      return func(args...) {
      global call_count
      call_count += 1
      
      print("[METRICS] Recording call to ", metric_name)
      
      result = f(...args, **kwargs)
      
      print("[METRICS] Execution time recorded")
      print("[METRICS] Function called ", call_count, " times")
      
      return result
    }
  }
}

print("=== Test 5: Metrics Collector ===")
@collect_metrics(metric_name="calculate")
func calculate(value) {
  print("Calculating: ", value)
  return value * 2
}

result5 = calculate(100)
print("Result: ", result5)

# Test 6: Conditional execution
func conditional(condition=true, opts**) {
  return func(f) {
      return func(args...) {
      # Check if we should execute
      should_execute = opts.exists("enabled") and opts["enabled"]
      
      print("[CONDITIONAL] Checking condition: enabled=", should_execute)
      
      if not should_execute {
        print("[CONDITIONAL] Condition not met, skipping")
        return null
      }
      
      print("[CONDITIONAL] Condition met, executing function")
      return f(...args)
    }
  }
}

print("=== Test 6: Conditional Execution ===")
@conditional(enabled=true)
func feature_flag_function() {
  print("Feature enabled - executing")
}

feature_flag_function()

# Test 7: Batch processor decorator
func batch_process(batch_size=10, opts**) {
  return func(f) {
    return func(items, args..., kwargs**) {
      print("[BATCH] Processing batch of ", len(items), " items")
      
      results = []
      for item in items {
        result = f(item, ...args, **kwargs)
        results.append(result)
      }
      
      print("[BATCH] Batch completed successfully")
      return results
    }
  }
}

print("=== Test 7: Batch Processor ===")
@batch_process(batch_size=100)
func process_item(item) {
  print("Processing item: ", item)
  return item + "_processed"
}

items = ["item1", "item2", "item3"]
process_item(items)

