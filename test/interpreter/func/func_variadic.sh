# function variadic test
# --output:start
# 12alex
# 512alex
# --output:end

func f(t...) {
  print(t[0], t[1], t[2])
}

func f2(value, t...) {
  print(value, t[0], t[1], t[2])
}

alex = "alex"

f(1, 2, "alex")
f2(5, 1, 2, alex)
