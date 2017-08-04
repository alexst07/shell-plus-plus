# function parameters test
# --output:start
# 1(2, 3, 4)50
# 1(2, 3, 4)10
# 1(2, 3, 4, 5)10
# 1()10
# [a, b, c]
# [c, b]
# 10520
# 10510
# 10210
# 1023
# 1023
# --output:end

func test(a, c..., d=10) {
  print(a, c, d)
}

func test2(a, b=5, c=10) {
  print(a,b,c)
}

test(1, 2, 3, 4,d= 50)
test(1, 2, 3, 4)
test(1, 2, 3, 4, 5)
test(1)

print(test2.__params__)
print(test2.__default_params__)

test2(10, c = 20)
test2(10)
test2(10, b = 2)
test2(a=10, b = 2, c= 3)
test2(10, b = 2, c= 3)
