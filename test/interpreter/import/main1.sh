# simple circular import test
# --output:start
# true
# change here
# asdfs
# change here
# teste
# true
# true
# --output:end

str = "teste"

import "import1.sh" as imp

print(imp.my_var)
print(imp.str)
print(str)

print(str instanceof string)
print(imp.my_var instanceof string)
