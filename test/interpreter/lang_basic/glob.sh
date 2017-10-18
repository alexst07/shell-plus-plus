# glob test
# --output:start
# a.txt b.txt c.txt
# *.txt
# *.err
# adir bdir cdir testdir
# bdir
# *dir
# --output:end

files = ["a.txt", "b.txt", "c.txt", "file"]
dirs = ["adir", "bdir", "cdir", "testdir"]

mkdir test
cd test

for f in files {
  touch ${f}
}

for d in dirs {
  mkdir ${d}
}

echo *.txt
echo "*.txt"
echo *.err
echo *dir
echo bdir
echo "*dir"

for f in files {
  rm ${f}
}

for d in dirs {
  rm -rf ${d}
}

cd ..
rm -rf test
