# declaring a int variable and printing it
x = 10
print("the value of x is ", x)

# changing the value of a variable
x = 20
print("now the value of x is ", x)

# string variable
name = "alex"
print("my name is ", name)

# float variable
pi = 3.14
print("the value of PI is ", pi)

# bool
var1 = true
var2 = false
print("var1 is ", var1, " and, var2 is ", var2)
print("var1 is equal v2? ", var1 == var2)

a, b, c, d = 4, 4.5, "string", true
print(a, " ", b, " ", c, " ", d)

#
# let expression
#

# let keyword allow handle an assignment as an expression
# and let return the same object that was assigned to the variables
x = let y = (let w = 15) - 4
print("x: ", x, " y: ", y, " w: ", w)

# other example
print(">> ", let q = "hello")
print("q: ", q)

# with tuple
a, b = let x, y = 1, 5
print("a: ", a, " b: ", b)
print("x: ", x, " y: ", y)
