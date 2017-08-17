# while break test
# --output:start
# 4950
# --output:end

i = 0
sum = 0
while true {
  if i == 100 {
    break
  }

  sum += i
  i += 1
}

assert(sum == 4950, "error on sum")
print(sum)
