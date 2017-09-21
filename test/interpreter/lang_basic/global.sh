# if else test
# --output:start
# a: local func
# b: global func
# a: local test
# b: global func
# start func2
# x local
# y global
# x call in func2
# y call in func2
# start func1
# x local
# y call in func2
# x call in func1
# y call in func1
# end func1
# x call in func2
# y call in func1
# end func2
# x local
# y call in func1
# --output:end

a = "local test"
global b = "global test"

func test() {
  b = "global func"
  a = "local func"
  print("a: ", a)
  print("b: ", b)
}

test()
print("a: ", a)
print("b: ", b)

global y = "y global"
x = "x local"

func func1() {
  print("start func1")
  print(x)
  print(y)
  y = "y call in func1"
  x = "x call in func1"
  print(x)
  print(y)
  print("end func1")
}

func func2() {
  print("start func2")
  print(x)
  print(y)
  y = "y call in func2"
  x = "x call in func2"
  print(x)
  print(y)
  func1()
  print(x)
  print(y)
  print("end func2")
}

func2()

print(x)
print(y)
