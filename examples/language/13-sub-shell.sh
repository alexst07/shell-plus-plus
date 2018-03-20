# subshell works like a command in place, it means, it create another process
# too, like the commands do.
# every operation you do with command you can do with subshell, for example
# pipeline subshell output to other command
shell {
  print("line one")
  print("line two")
  print("other")
} | grep line

# you can output to file using subshell
shell {
  print("line stdout")
  print_err("line stderr")
} > test.txt 2> test_err.txt

cat < test_err.txt
cat < test.txt

rm test_err.txt test.txt

# variables changed inside subshell don't affect outside subshell
my_var = 5
shell {
  print("in subshell: my_var: ", my_var)
  my_var = 8
  print("in subshell: my_var: ", my_var)
}

print("out subshell: my_var: ", my_var)

# like commands, you can do &&(and) and ||(or) operations with subshell

# executes shell A and shell B
shell {
  echo shell A
  exit 0
} && shell {
  echo shell B
  exit 0
}

# executes only shell A
shell {
  echo shell A
  exit 0
} || shell {
  echo shell B
  exit 0
}

# executes only shell A
shell {
  echo shell A
  exit 1
} && shell {
  echo shell B
  exit 0
}

# executes shell A and shell B
shell {
  echo shell A
  exit 1
} || shell {
  echo shell B
  exit 0
}
