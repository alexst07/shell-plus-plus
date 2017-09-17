# if else test
# --output:start
# [2, 2, 4, 4]
# [1, 2, 2, 3, 3, 4, 4, 5]
# [[1, 2], [2, 3], [3, 4], [4, 5]]
# [1, 4, 9, 16]
# [9, 16]
# --output:end


a = [[1,2], [2,3], [3,4], [4,5]]
b = [b for i in a for b in i if b % 2 == 0]
print(b)

b = [b for i in a for b in i]
print(b)

b = [i for i in a for b in i if b % 2 == 0]
print(b)

x = [1, 2, 3, 4]
y = [i*i for i in x]
print(y)

x = [1, 2, 3, 4]
y = [i*i for i in x if i > 2]
print(y)
