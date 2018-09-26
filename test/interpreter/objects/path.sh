# path test
# --output:start
# /home/alex
# /home/alex/books
# true
# --output:end

p1 = path("/home")
p_books = path("books")

# test path concatanation
p2 = p1 / "alex"
p3 = p2 / p_books

print(p2)
print(p3)

# test path comparation
pwd1 = path.pwd()
pwd2 = path.pwd()
print(pwd1 == pwd2)