# import test
# --output:start
# hello
# alex
# --output:end

import "import.sh" as imp

imp.test()

t = imp.Test("alex")
print(t)
