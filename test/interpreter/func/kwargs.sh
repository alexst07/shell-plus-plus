# Test kwargs functionality
# --output:start
# Test 1: Basic kwargs
# a: 1
# b: 2
# opts: {"x": 10, "y": 20, "z": 30}
# Test 2: Kwargs with defaults
# a: 1 b: 5 opts: {"x": 10}
# a: 1 b: 3 opts: {"x": 10, "y": 20}
# Test 3: Empty kwargs
# a: 1 b: 2 opts: {}
# Test 4: Kwargs only
# opts: {"a": 1, "b": 2, "c": 3}
# Test 5: Kwargs with named regular params
# a: 100 b: 200 opts: {"x": 10, "y": 20}
# --output:end

# Test 1: Basic kwargs
func test_kwargs(a, b, opts**) {
  print("a:", a)
  print("b:", b)
  print("opts:", opts)
}

print("Test 1: Basic kwargs")
test_kwargs(1, 2, x=10, y=20, z=30)

# Test 2: Kwargs with defaults
func test_kwargs_defaults(a, b=5, opts**) {
  print("a:", a, "b:", b, "opts:", opts)
}

print("Test 2: Kwargs with defaults")
test_kwargs_defaults(1, x=10)
test_kwargs_defaults(1, b=3, x=10, y=20)

# Test 3: Empty kwargs
print("Test 3: Empty kwargs")
test_kwargs(1, 2)

# Test 4: Kwargs only
func test_kwargs_only(opts**) {
  print("opts:", opts)
}

print("Test 4: Kwargs only")
test_kwargs_only(a=1, b=2, c=3)

# Test 5: Kwargs with named regular params
print("Test 5: Kwargs with named regular params")
test_kwargs(a=100, b=200, x=10, y=20)

