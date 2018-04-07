# import with id list
# --output:start
# local test
# test
# class A
# --output:end

func Test() {
  print("local test")
}

Test()

import A, Test from "module.sh"

Test()

a = A("class A")
print(a.get())
