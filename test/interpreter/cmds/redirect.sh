# execute simple cmds
# --output:start
# world
# hello
#
# file: test_out.txt
# hello
#
# file: test_err.txt
# world
#
# file: test_all.txt
# hello
# world
#
# file: test2_out.txt
# hello
# world
# --output:end

cmd test {
  print("hello")
  print_err("world")
}

cmd test2 {
  test 1>&2
}

test > test_out.txt
test 2> test_err.txt
test &> test_all.txt
echo
print("file: test_out.txt")
cat < test_out.txt
echo

print("file: test_err.txt")
cat < test_err.txt
echo

print("file: test_all.txt")
cat < test_all.txt
echo

test2 2> test2_out.txt
print("file: test2_out.txt")
cat < test2_out.txt

rm test_all.txt test_err.txt test_out.txt test2_out.txt
