# Test map unpacking with ** in function calls
# --output:start
# === Test 1: Basic Map Unpacking ===
# Greeting: Hello
# Name: Alice
# === Test 2: Mix Positional and Map Unpack ===
# Host: localhost Port: 8080 Debug: true
# === Test 3: Multiple Map Unpacks ===
# Timeout: 30 Retries: 5 Verbose: false
# === Test 4: Combine All Argument Types ===
# a: 1 b: 2 args: (3, 4, 5) opts: {(z, 30), (y, 20), (x, 10)}
# === Test 5: Map Unpack with Named Args ===
# Server: myserver Port: 3000 SSL: true Timeout: 60
# === Test 6: Empty Map ===
# Name: Bob Greeting: Hi
# --output:end

# Test 1: Basic map unpacking
func greet(name, greeting="Hello") {
  print("Greeting: ", greeting)
  print("Name: ", name)
}

print("=== Test 1: Basic Map Unpacking ===")
options = {"name": "Alice", "greeting": "Hello"}
greet(**options)

# Test 2: Mix positional and map unpack
func configure(host, port, debug=false) {
  print("Host: ", host, " Port: ", port, " Debug: ", debug)
}

print("=== Test 2: Mix Positional and Map Unpack ===")
settings = {"port": 8080, "debug": true}
configure("localhost", **settings)

# Test 3: Multiple map unpacks (later wins)
func setup(timeout=10, retries=1, verbose=false) {
  print("Timeout: ", timeout, " Retries: ", retries, " Verbose: ", verbose)
}

print("=== Test 3: Multiple Map Unpacks ===")
defaults = {"timeout": 30, "retries": 3}
overrides = {"retries": 5}
setup(**defaults, **overrides)  # retries from overrides wins

# Test 4: Combine all argument types
func complex(a, b, args..., opts**) {
  print("a: ", a, " b: ", b, " args: ", args, " opts: ", opts)
}

print("=== Test 4: Combine All Argument Types ===")
extra_args = [3, 4, 5]
extra_opts = {"x": 10, "y": 20}
complex(1, 2, ...extra_args, **extra_opts, z=30)

# Test 5: Map unpack with named args
func connect(server, port=80, ssl=false, timeout=30) {
  print("Server: ", server, " Port: ", port, " SSL: ", ssl, " Timeout: ", timeout)
}

print("=== Test 5: Map Unpack with Named Args ===")
config = {"port": 3000, "ssl": true}
connect("myserver", **config, timeout=60)

# Test 6: Empty map
print("=== Test 6: Empty Map ===")
empty = {}
greet("Bob", "Hi", **empty)

