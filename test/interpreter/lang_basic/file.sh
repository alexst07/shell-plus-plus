# while break test
# --output:start
# hello world
#
# lo wo
# 8
# ello world
#
# line: line1
# line: line2
# line: line3
# line: line4
# line-> it_line1
# line-> it_line2
# line-> it_line3
# line-> it_line4
#
# --output:end

echo hello world > hello.txt

f = file("hello.txt")
print(f.read())
f.seekg(3)
print(f.read(5))

print(f.tellg())
f.seekg(1)
print(f.readall())

echo line1 > other.txt
echo line2 >> other.txt
echo line3 >> other.txt
echo line4 >> other.txt

q = file("other.txt")
while true {
  r = q.readline()
  if !r {
    break
  }
  print("line: ", r)
}

echo it_line1 > iter.txt
echo it_line2 >> iter.txt
echo it_line3 >> iter.txt
echo it_line4 >> iter.txt

for line in file("iter.txt") {
  print("line-> ", line)
}

rm iter.txt hello.txt other.txt
