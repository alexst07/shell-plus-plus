# function default parameter test
# --output:start
# 87alex
# 7testalex
# 7xal
# --output:end

func f(i, a = 5 + 2, b = "alex") {
  print(i, a, b)
}

f(8)
f(7, "test")
f(7, "x", "al")
