# execute let expression test
# --output:start
# a: true
# 46
# test1
# test2
# test3
# --output:end

if let a = true {
  print("a: ", a)
}

a = let x, y = 4, 6

print(a[0], a[1])

echo "test1" > test.txt
echo "test2" >> test.txt
echo "test3" >> test.txt

shell {
  while let r = read() {
    print(r)
  }
} < test.txt

rm test.txt
