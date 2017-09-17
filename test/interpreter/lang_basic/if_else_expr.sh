# if else test
# --output:start
# 9
# 2
# first
# first
# first
# second
# second
# --output:end

a1 = 8

b = 9 if a1 > 5 else 2
print(b)

b = 9 if a1 < 5 else 2
print(b)

c = $(echo first) if a1 == 8 else $(echo second)
print(c)

c = $(echo first) if a1 == 8
      else $(echo second)
print(c)

c = $(echo first) if a1 == 8 else $(echo second)
print(c)

c = $(echo first) if
      a1 != 8 else $(echo second)
print(c)

c = $(echo first) if a1 != 8 else
    $(echo second)
print(c)
