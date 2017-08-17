# execute sub shell test
# --output:start
# 123
# change
# alex
# uuu
# asdf
# yyy
# test
# --output:end


shell {
  echo 123
} > test.tx

cat test.tx
rm test.tx

t = "alex"
shell {
  t = "change"
  echo ${t}
}

echo ${t}

shell {
  echo uuu
} | cat

echo asdf | shell {
  print(read())
}

print("yyy")
echo test
