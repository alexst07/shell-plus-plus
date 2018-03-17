files_type = {"jpg": "image file type",
    "cpp": "cpp code",
    "wav": "audio file type",
    "sh": "shell script file type"}

# key access
print("files_type[\"cpp\"]")
print("-> ", files_type["cpp"], "\n")

files_type["cpp"] = "c plus plus file type"
print("files_type[\"cpp\"]")
print("-> ", files_type["cpp"], "\n")

# keys operation
print("files_type.keys()")
print("-> ", files_type.keys())
print()

# values operation
print("files_type.values()")
print("-> ", files_type.values())
print()

# exists operation
print("files_type.exists(\"sh\")")
print("-> ", files_type.exists("sh"))
print()

print("files_type.exists(\"shpp\")")
print("-> ", files_type.exists("shpp"))
print()

# append operation
files_type += {"avi": "video file type"}
print("files_type.update...")
print("-> ", files_type)

# del operator
del files_type["sh"]
print("del files_type[\"sh\"]")
print("-> ", files_type)
