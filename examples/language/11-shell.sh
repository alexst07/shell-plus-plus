#
# Basic commands
#

echo Hello World
ls -l

# redirect commands output and input

# create file test.txt and insert echo output
echo this is my line > test.txt

# append echo output on test.txt
echo this is my other line >> test.txt

# redirect test.txt to input of command cat
cat < test.txt

# remove test.txt
rm test.txt

#
# executing expression inside shell commands
#

# to execute an expression as commands arguments, you have to use ${}
# print a simple expression with echo command
my_var = "this is a test"
echo ${my_var}
echo ${my_var.to_upper()}

# an expression can be passed on redirect too
my_file = "other.txt"
echo my line > ${my_file}
echo my other line >> ${my_file}
echo ${my_var} >> ${my_file}
cat < ${my_file}
rm ${my_file}

# commands pipeline
echo first line > grep.txt
echo second line >> grep.txt
echo last part >> grep.txt
cat < grep.txt | grep part
cat < ${"grep.txt"} | grep ${"line"}
cat < ${"grep.txt"} | grep ${"part"} > line.txt
cat < line.txt
rm line.txt

# redirect from variable
my_var = "test redirect\n"
cat << ${my_var}

# command as an expression
echo = $(echo my test)
print(echo)

# command expression is an object of type cmdobj
print(type(echo))

# convert cmdobj to string
str = string(echo)
print(str)

# iterate over a cmdobj with for ... in statement
for line in $(cat < grep.txt) {
  print("line: ", line)
}

# convert cmdobj to array
c = $(cat < grep.txt)
arr = array(c)
print(arr[1])

rm grep.txt
