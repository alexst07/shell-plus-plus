# execute let expression test
# --output:start
# counter: 0
# exec p
# counter: 1
# exec p
# counter: 2
# exec p
# counter: 3
# exec p
# counter: 4
# exec p
# counter: 5
# --output:end

counter = 0

func test() {
  if counter == 5 {
    print("counter: ", counter)
    return false
  } else {
    print("counter: ", counter)
    counter += 1
    return true
  }
}

while let r = test() {
  print("exec p")
}
