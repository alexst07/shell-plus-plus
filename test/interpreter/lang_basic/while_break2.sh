# while break test
# --output:start
# a: 0
# a: 1
# a: 2
# a: 3
# a: 4
# 3
# --output:end

i = 0
sum = 0
while true {
  if i == 3 {
    break
  }

  sum += i
  i += 1
}

a = 0
while a < 5 {
  print("a: ", a)
  a += 1
}

print(sum)
