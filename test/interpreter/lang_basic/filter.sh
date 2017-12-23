# filter test
# --output:start
# [15, 12, 14, 47]
# [8, 2, 12, 2, 14]
# dic1
# key: key4, value: 11
# key: key6, value: 15
# key: key5, value: 12
# dic2
# key: key1, value: 2
# key: key3, value: 6
# key: key5, value: 12
# --output:end

arr = [1, 3, 8, 2, 5, 9, 15, 12, 2, 14, 47]
arr2 = copy(arr)

arr.filter(lambda x: x > 10)
arr2.filter(lambda x: x % 2 == 0)
print(arr)
print(arr2)

dic = {"key1": 2, "key2": 3, "key3": 6, "key4": 11, "key5": 12, "key6": 15}
dic2 = copy(dic)

dic.filter(lambda k, v: v > 10)
dic2.filter(lambda k, v: v % 2 == 0)

print("dic1")
for k, v in dic {
  print("key: ", k, ", value: ", v)
}

print("dic2")
for k, v in dic2 {
  print("key: ", k, ", value: ", v)
}
