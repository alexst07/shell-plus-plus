# string variable
str_var1 = "my string"
str_var2 = "nice"
# concatenate string
str_concat = str_var1 + " is " + str_var2

# print string variable
print("my string is: ", str_concat)

# slicing string
abc = "abcdefghijklmnopqrstuvwxyz"
print(abc)
print(abc[20:])
print(abc[0:3])
print(abc[-5:])
print(abc[20:-2])
print(abc[-4:-2])

# string position get
print(abc[1])
print(abc.at(2))
print(abc[0] + " ... " + abc[-1:])

# string size
print("abc has ", len(abc), " letters")

# upper case and lower case
name = "Grüssen"
print("upper name: " + name.to_upper())
print("lower name: " + name.to_lower())

# trim opration
trim = " \t shell script \t "
trim_l = " \t shell script \t "
trim_r = " \t shell script \t "
print("_" + trim_l.trim_left() + "_")
print("_" + trim_r.trim_right() + "_")
print("_" + trim.trim() + "_")

# find operation
statement = "hello for everyone in the room"
f_str = "for"
pos = statement.find(f_str)
print("found in position: ", pos)

f_str = "not"
pos = statement.find(f_str)
print("found? ", pos)

# replace operation
statement.replace("hello", "hy")
print("new statement: ", statement)

str_ex = "cat and echo and grep and others"
print(str_ex)
str_ex.replace_first("and", ",")
print(str_ex)
str_ex.replace_last("and", ",")
print(str_ex)

# erase all operation
str_ex2 = "cat and echo and grep and others"
print(str_ex2.erase_all("and "))

# count operation
str_ex3 = "cat and echo and grep and others"
print("the sentece has ", str_ex3.count("and"), " \"and\" word")
