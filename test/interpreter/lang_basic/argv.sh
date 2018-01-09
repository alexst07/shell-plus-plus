# test bug when import come before sys.argv
# --output:start
# 123
# [../test/interpreter/lang_basic/argv.sh]
# --output:end

import "import-test.sh" as test

print(test.test_var)
print(sys.argv)
