# Simple decorator test with kwargs
# --output:start
# [CONFIG] level: DEBUG, verbose: true
# add called with: 5, 10
# Result: 15
# --output:end

func configured_decorator(level="INFO", config**) {
  return func(f) {
    return func(args...) {
      verbose = config.exists("verbose") and config["verbose"]
      
      if verbose {
        print("[CONFIG] level: ", level, ", verbose: ", config["verbose"])
      }
      
      result = f(...args)
      return result
    }
  }
}

@configured_decorator(level="DEBUG", verbose=true)
func add(x, y) {
  print("add called with: ", x, ", ", y)
  return x + y
}

result = add(5, 10)
print("Result: ", result)

