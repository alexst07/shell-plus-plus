# Simple test for map unpacking
# --output:start
# Greeting: Hello
# Name: Alice
# --output:end

func greet(name, greeting="Hello") {
  print("Greeting: ", greeting)
  print("Name: ", name)
}

options = {"name": "Alice", "greeting": "Hello"}
greet(**options)

