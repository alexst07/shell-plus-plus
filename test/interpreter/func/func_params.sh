# variables with same in different scopes
# --output:start
# 3:6
# 10:20
# --output:end

func test(a, b) {
  print(a, ":", b)
}

a = 10
b = 20

test(3, 6)
print(a, ":", b)
