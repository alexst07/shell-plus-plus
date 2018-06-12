cmds = ["echo", "cat", "ls", "rm", "cd"]

# slice arrays
print("cmds[1] -> ", cmds[1])
print("cmds[1:] -> ", cmds[1:])
print("cmds[1:3] -> ", cmds[1:3])
print("cmds[:3] -> ", cmds[:3])
print("cmds[0:-1] -> ", cmds[0:-1])
print("cmds[-2:] -> ", cmds[-2:])

# append
cmds.append("chmod")
print("cmds.append(\"chmod\") -> ")
print("-> ", cmds)
print()

# extends
cmds += ["grep"] + ["tr", "more"]
print("cmds += [\"grep\"] + [\"tr\", \"more\"] -> ")
print("-> ", cmds)
print()

cmds.extend(["tail"])
print("cmds.extend([\"tail\"]) -> ")
print("-> ", cmds)
print()

# insert
cmds.insert(1, "mv")
print("cmds.insert(1, \"mv\")")
print("-> ", cmds)
print()

# remove
cmds.remove("cat")
print("cmds.remove(\"cat\")")
print("-> ", cmds)
print()

# pop
cmds.pop(4)
print("cmds.pop(4)")
print("-> ", cmds)
print()

# index
print("cmds.index(\"mv\")")
print("-> ", cmds.index("mv"))
print()

# count
cmds += ["ls"]
print("cmds.count(\"ls\")")
print("-> ", cmds.count("ls"))
print()

# sort
print("cmds.sort()")
print("-> ", cmds.sort())
print()

# reverse
print("cmds.reverse()")
print("-> ", cmds.reverse())
print()

# map
print("cmds.map...")
cmds.map(lambda x: x.to_upper() if len(x) > 2 else x)
print("-> ", cmds)
print()

# filter
print("cmds.filter...")
cmds.filter(lambda x: true if len(x) == 2 else false)
print("-> ", cmds)
print()

# join
print("cmds.join...")
s = cmds.join("-")
print(s)
print("-> ", cmds)
print()

# del operator
del cmds[2]
print("del cmds[2]")
print("-> ", cmds)
print()

# clear
print("cmds.clear()")
cmds.clear()
print("-> ", cmds)
print()
