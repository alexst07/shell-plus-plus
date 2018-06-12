# A function is a block of organized, reusable code that is used to perform
# a single, related action. Functions provide better modularity for your
# application and a high degree of code reusing.

# Defining a simple function
func print_str(str) {
  print(str)
}

# calling a function
print_str("test function print_str")

# Pass argument by value
func not_change(i) {
  i = 50
  print("in func: i: ", i)
}

i = 20
not_change(i)
print("out func: i: ", i)

# Pass argument by reference
# arguments like array, tuple or instance of some class is passed always by
# reference, so what you change inside function, change outside function too
func change(a) {
  a.append([1, 5])
  print("in func change(): a: ", a)
}

a = [1, 2, 3]
print("before call change(): a: ", a)
change(a)
print("after call change(): a: ", a)

#
# Function Arguments
#

# You can call a function by using the following types of formal arguments
# - Required arguments
# - Keyword arguments
# - Default arguments
# - Variable-length arguments

# Required arguments
#
# Required arguments are the arguments passed to a function in correct
# positional order. Here, the number of arguments in the function call
# should match exactly with the function definition.

func test(str) {
  print(str)
}

test("test")

# Keyword arguments
#
# Keyword arguments are related to the function calls.
# When you use keyword arguments in a function call,
# the caller identifies the arguments by the parameter name.

func print_info(name, age) {
  print("Name: ", name)
  print("Age: ", age)
}

print_info(age=25, name="abcd")

# Default arguments
#
# A default argument is an argument that assumes a default value
# if a value is not provided in the function call for that argument.
func print_info_default(name, age = 28) {
  print("Name: ", name)
  print("Age: ", age)
}

print_info_default(name="dcba", age=75)
print_info_default(name="dcba")

# Variable-length arguments
#
# You may need to process a function for more arguments than you
# specified while defining the function.
func print_args(arg0, my_args...) {
  print("arg0: ", arg0)
  print("len(my_args): ", len(my_args))

  for arg in my_args {
    print("arg: ", arg)
  }
}

print_args("arg0", 5, "test", [4, 9])
