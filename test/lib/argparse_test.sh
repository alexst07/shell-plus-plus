import ArgumentParser from "./../../lib/argparse.sh"

argparse = ArgumentParser()
argparse.add_argument("--test", "test")
argparse.process(sys.argv)
print(argparse.vars())
