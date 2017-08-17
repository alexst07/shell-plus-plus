# param error test
# --output:start
# Error: 6: 1: only last parameter can be variadic
# --output:end

func test(a, c..., d) {
  print(a)
}

test(5, 6, 8, 9)
