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
