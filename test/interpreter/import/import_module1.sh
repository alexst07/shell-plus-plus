# import with star
# --output:start
# my A
# local test
# test
# class A
# class B
# --output:end

func A() {
  print("my A")
}

A()

func Test() {
  print("local test")
}

Test()

import * from "module.sh"

Test()

a = A("class A")
print(a.get())

b = B("class B")
print(b.get())
