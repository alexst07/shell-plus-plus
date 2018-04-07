# import with as keyword
# --output:start
# test
# class A
# class B
# --output:end

import "module.sh" as mod

mod.Test()

a = mod.A("class A")
print(a.get())

b = mod.B("class B")
print(b.get())
