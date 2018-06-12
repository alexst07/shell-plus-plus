### while statement ###
#
# A while statement executes its statements as long as a specified condition
# evaluates to true.

# The following while loop iterates as long as n is less than three:
n = 0
x = 0

while n < 3 {
  n += 1
  x += n
  print("value of x: ", x)
}

### break statement ###
#
# Use the break statement to terminate a loop, switch, or in conjunction with
# a labeled statement.

# The following example iterates through the elements in an array until it
# finds the index of an element whose value is the_value:
a = [1, 3, 8, 5, 2, 4]
i = 0
while i < len(a) {
  if a[i] == 5 {
    break
  }

  print("while: value o a[", i, "]: ", a[i])
  i += 1
}

### continue statement ###
#
# The continue statement can be used to restart a while or for in loop

# The following example shows a while loop with a continue statement that
# executes when the value of i is three.
i = 0
while i < 5 {
  i += 1break

  if i == 3 {
    continue
  }

  print("value of i: ", i)
}

### for...in statement
#
# The for...in statement creates a loop iterating over iterable objects
# (including array, map, tuple, and others iterable objects)

arr = ["cat", "mv", "ls", "rm"]

for a in arr {
  print(": ", a)
}

for a in arr {
  if a == "ls" {
    break
  }

  print(":: ", a)
}

for a in arr {
  if a == "ls" {
    continue
  }

  print("> ", a)
}

#
# List comprehension
#
files = [file for file in $(ls) if file.size("k") > 10]
print(files)
