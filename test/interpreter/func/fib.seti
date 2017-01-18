# fibnacci test
# --output:start
# 55
# --output:end

func f(i) {
  if i < 2 {
    return i
  } else {
    return f(i-1) + f(i - 2)
  }
}

res = f(10)
assert(res == 55, "error on fib result")

print(res)
