# Test combined variadic and kwargs functionality
# --output:start
# Test 1: Both variadic and kwargs
# a: 1
# b: 2
# args: (3, 4, 5)
# opts: {"x": 10, "y": 20}
# Test 2: Variadic with kwargs, no extra args
# a: 1
# b: 2
# args: ()
# opts: {"x": 10, "y": 20}
# Test 3: Variadic with kwargs, no extra kwargs
# a: 1
# b: 2
# args: (3, 4, 5)
# opts: {}
# Test 4: All parameters
# a: 1
# b: 2
# c: 100
# args: (3, 4, 5)
# opts: {"x": 10, "y": 20}
# --output:end

# Test 1: Both variadic and kwargs
func test_all(a, b, args..., opts**) {
  print("a:", a)
  print("b:", b)
  print("args:", args)
  print("opts:", opts)
}

print("Test 1: Both variadic and kwargs")
test_all(1, 2, 3, 4, 5, x=10, y=20)

# Test 2: Variadic with kwargs, no extra variadic args
print("Test 2: Variadic with kwargs, no extra args")
test_all(1, 2, x=10, y=20)

# Test 3: Variadic with kwargs, no extra kwargs
print("Test 3: Variadic with kwargs, no extra kwargs")
test_all(1, 2, 3, 4, 5)

# Test 4: With default parameter between regular and variadic
func test_with_default(a, b, c=100, args..., opts**) {
  print("a:", a)
  print("b:", b)
  print("c:", c)
  print("args:", args)
  print("opts:", opts)
}

print("Test 4: All parameters")
test_with_default(1, 2, 3, 4, 5, x=10, y=20)

