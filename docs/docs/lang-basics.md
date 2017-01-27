# Language basics

## Hello World

In seti there are several ways to print information on screen, you can use shell
commands or the function print.


Using shell commands:
```sh
echo Hello World
```

Using seti print function:
```go
print("Hello World")
```
## Comments
In seti comments are everything that comes after #
```python
# comment
print("hello world") # print hello world
```

## Variables
```go
my_var = 7
```
if my_var doesn't exists, seti will create my_var and attributes 7 to it,
if it already exists, the value 7 will be attributed to my_var

## Basic types
### integer
```go
my_var = 7
print(type(my_var))  # int
```

Like in other languages, int in seti has oprations as add, sub, div and mult

```go
v1 = 7
v2 = 3
v3 = v1 + v2*v1 - v2/v1
v3 += v1%v2 # mod
```

### real
```go
my_var = 7.5
print(type(my_var))  # real
```
Like in other languages, real in seti has oprations as add, sub, div and mult
```go
v1 = 7.1
v2 = 3.3
v3 = v1 + v2*v1 - v2/v1
v3 += 2.3
```

### string
```go
my_var = "test"
print(type(my_var))  # string
```

Concatenating string:
```go
str1 = "hello"
str2 = "world"
str = str1 + " " + str2
```
String functions:
string.upper()
string.lower()
string.split(delim)
string.trim(arg = " ")
string.trim_left(arg = " ")
string.trim_right(arg = " ")

```go
str1 = "hElLo"
str2 = "hElLo"
str1.upper() # HELLO
str2.lower() # hello
```

```go
str1 = "test1,test2,test3"
arr = str1.split(",")  # ["test1", "test2", "test3"]
```
```go
str1 = "  test1  "
str2 = "  test2  "
str1.trimm_right()  # "  test1"
str1.trimm_left()  # "test1"
str2.trim()  # "test2"
```

### null
```go
my_var = null
print(my_var)  # null
print(type(my_var))  # null_t
```
### boolean
```go
my_var_t = true
my_var_f = false
print(my_var_t)  # true
print(type(my_var_t))  # bool
```

### array
```go
my_var = [4, "str", [1, 2]]
print(type(my_var))  # array
```

Array access
```go
arr = [4, "str", [1, 2]]
print(arr[0])  # 4
print(arr[1])  # str
print(arr[2][1])  # 2

arr[0] = 10
print(arr[0])  # 10
```
Array functions:
array.join(sep)

### map
```go
my_var = {"key1": "test", "key2": 5, 6:"other"}
print(type(my_var))  # map
```

Map access
```python
my_var = {"key1": "test", {"key2": 5, "t":[1,2]}, 6:"other"}
print(arr["key1"])  # "test"
print(arr["key2"]["t"])  # [1, 2]
print(arr["key2"]["t"][0])  # 1

my_var["key1"] = 4
print(arr["key1"])  # 4
```
### tuple
```go
my_var = "str", 4, [1, 3]
print(type(my_var))  # tuple
```
Tuple access works as array access

## Control flow statements
### If and else

```go
if is_raining() {
  print ("it is raining")
} else if is_snowing() {
  print("it is snowing")
} else {
  print("sun is up in the sky")
}
```

### Switch case

```go
a = "res"

switch a {
  case "asdf",7 {
    print("ok")
  }

  case "res" {
    print("other")
  }

  default {
    print("default")
  }
}
```

if switch has no argument, so it is like the argument was true
```go
switch {
  case true {
    print("true")
  }

  case false {
    print("false")
  }
}
```

### While loop
```go
a = 1

while a < 10 {
  print(a)
  a = a + 1
}
```

### For loop
```go
a = [1, 2, 3, 4]

for i in a {
  print(i)
}
```
print:
```
1
2
3
4
```

```go
a = [1, 2, 3, 4]
b = [10, 11, 12, 13, 15, 16]
c = [6, 5, 7]

for i in a, b, [55, 56, 57] {
  print(i)
}
```
print:
```
(1, 10, 55)
(2, 11, 56)
(3, 12, 57)
```

```go
v1 = [1, 2, 3, 4]
v2 = [10, 11]

for i, j in v1, v2 {
  print(i, ": " ,j)
}
```

print:
```
1:10
2:11
```

### Break and continue
```go
i = 0
while i < 10 {
  if i == 7 {
    break
  }

  print(i)
  i = i +1
}
```
print:
```
0
1
2
3
4
5
6
```

```go
i = 0
while i < 10 {
  if i == 7 {
    i = i +1
    continue
  }

  print(i)
  i = i +1
}
```
print:
```
0
1
2
3
4
5
6
8
9
0
```

## Shell commands

To execute others programs in seti is very similar the way that works others
shell languages.

### Simple command
```bash
echo hello world
```

### Redirect to a file
```bash
echo hello world > file.txt
```

#### Redirect and append to a file
```bash
echo hello world > file.txt
echo append this text >> file.txt
```

#### Input from a file
```bash
cat < file.txt
```

### Pipeline of commands
```bash
ls | grep test
```
```bash
cat < file.txt | grep mytext
```
```bash
cat < file.txt | grep other_text > other.txt
```
### Logic operation with commands
And operation:
```bash
cmd1 && cmd2
```
If cmd1 has exit status 0 (correct), executes cmd2, if not, doesn't execute cmd2

Or operation:
```bash
cmd1 || cmd2
```
If cmd1 has exit status 0 (correct), doesn't execute cmd2, if not, executes cmd2

### Background commands
As in bash, to put a command in background in seti, the command must end with &

```bash
ls &
```
```bash
ls | grep some &
```

```bash
ls | grep some > file &
```

```bash
cmd1 || cmd2 &
```
### Assign commands to variables
Almost everything in seti is a an object, in fact only commands are not
objects, but the result of commands can be stored on an object

```bash
files = $(ls)
```

```bash
files = $(ls | grep test)
```

```bash
files = $(cat < file.txt | grep test)
```
the only requirement is that the output must be for variable, so it won't
work:

```bash
files = $(cat < file.txt | grep test > out.txt)
```

The variable that receives a command result can be used on if or for loop

```go
my_cmd = $(cmd_test)
if my_cmd {
  echo command worked
}
```

command object has an iterator object associated with it too:
```go
files = $(ls)

for file in files {
  print("file is: ", file)
}
```

or you can change the delimiter using in command iterator:

```go
for piece in $(cat < file).delim("\t") {
  print("piece is: ", file)
}
```

## Functions
In almost everything is an object, in fact except commands declaration,
everything is an ojbect including functions, so a function can be assigned
to other variable or be used as an argument for other function.
PS: Functions are NOT commands like happens in bash

### functions return
```go
func test(a, b) {
  print(a,b)
}
```
if no return is specified, the return of the function is null

```go
func test(a, b) {
  return a + b
}
```
### default parameters
the function can have default parameters:

```go
func test(a, b = 4) {
  return a + b
}

res = func(5, 6)  # 11
res1 = func(5)  # 9
```
### variadic
in seti functions support variadic parameters too:

```go
func make_and_print_tuple(t...) {
  for item in t {
    print("item: ", item)
  }
  return t
}

tres = make_and_print_tuple(5, "test", true)

```
### function as argument
```go
func receiv_func(a, b) {
  return b(a)
}

func test_b(i) {
  return i + 10
}

print(receiv_func(4,test_b))

```

### lambda functions
Lambda functions works almost as all other functions, it means, lambda
support default parameters and variadic, the difference is that, lambda
functions can be declared inside blocs, loop, if, or other function

```go
func ret_func(b) {
  a = func (i) {
    return i*b
  }

  return a
}

v = ret_func(5)(2)
print(v)  # 10
```

## Declaration of commands
Commands are the only thing in seti that is not an object, it means, you can't
assign a command to a variable

```go
cmd test {
  echo test has ${len(args)} arguments
  for arg in args {
    print("argument: ", arg)
  }
}

test -n -p alex
```
All argment is passed to command in a variable args, this variable is an array
of strings, commands can't be used as functions, so it won't work

```go
test("-n", "-p", "alex")  # it generates an error: test is not declared
```
