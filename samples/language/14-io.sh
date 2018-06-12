# read is a builtin function used to get input from stdin
# generally stdin is a keyboard input, but not always

# read a line from user input, and print the line
print("enter with a line: ", end="")
s = read()
print("line: ", s)

# using read you can do a command that read input from other command
cmd test {
  while let r = read() {
    print("test> ", r)
  }
}

echo line from echo | test

# or you can read a file with a command
echo line one > test.txt
echo line two >> test.txt
test < test.txt

# you can read from another declared command
cmd test2 {
  print("line stdout")
  print_err("line stderr")
}

test2 2>&1 > /dev/null | test

# read file from subshell
shell {
  while let r = read() {
    print("shell> ", r)
  }
} < test.txt

rm test.txt
