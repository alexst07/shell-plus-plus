# execute simple cmds
# --output:start
# hey,     hello world  there
# hello
# hey    hello
# hello   you
# hello   hello
# one|   |two
# --output:end

t = "hello"
q = ["one", "two"]
echo "hey,     ${t + \" world\"}  there"
echo "${t}"
echo "hey    ${t}"
echo "${t}   you"
echo "${t}   ${t}"
echo "${q[0]}|   |${q[1]}"
