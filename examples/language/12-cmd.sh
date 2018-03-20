# to declare your own command you use the keyword cmd
cmd test {
  print("test")
}

# so, you can use your command like any other
test

# the commands declared can handle arguments, like any other command
# all arguments passed by the user is elements from args array
# args is a variable the is created inside command symbol table
cmd test_with_args {
  print("args type: ", type(args))
  print("args len: ", len(args))

  for arg in args {
    print("> ", arg)
  }
}

# here is the test of the command with arguments
test_with_args --a arg1 --b arg2

# you can passa an array as arguments to command too
test_with_args $@{["arg1", "arg2"]}

# commands can output on stdout or stderr
# to print in stderr, just use print_err
cmd test_out {
  print("stdout: ", args.join(","))
  print_err("stderr: ", args.join(","))
}

# print stderr on file test_err.txt and discard stdout output
test_out 2> test_err.txt > /dev/null

# print stout on file test_out.txt and discard stdout stderr
test_out > test_out.txt 2> /dev/null

# show file test_err.txt
cat < test_err.txt

# show file test_out.txt
cat < test_out.txt

# remove files
rm test_err.txt
rm test_out.txt

# commands can be used with pipeline too
cmd test_lines {
  echo first line
  echo second line
  echo third
  echo other
}

# on this case grep will receive the std output from command test_lines
# grep will get the lines 'first line' and 'second line'
test_lines | grep line

# alias is a keyword used to give a nickname to a command with some arguments
alias grep = grep --color=auto

# now grep will show the lines with color
test_lines | grep line

cmd test_a {
  echo test_a executed
  exit 0
}

cmd test_b {
  echo test_b executed
  exit 0
}

cmd test_c {
  echo test_c executed
  exit 1
}

# as test_a return 0(no error) and test_b return 0(no error)
# && execute test_a and test_b
test_a && test_b

# as test_a return 0(no error) and test_b return 0(no error)
# && execute test_a only test_a, because test_b would be executed
# only if test_a had exited with error (different from 0)
test_a || test_b

# as test_c return 1(error) and test_a return 0(no error)
# && execute test_c, but it won't execute test_a
test_c && test_a

# as test_c return 1(error) and test_a return 0(no error)
# || execute test_c, and as test_c exited with error, so test_a will be
# executed
test_c || test_a
