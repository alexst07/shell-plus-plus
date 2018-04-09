class InvalidArgumentException(Exception) {
  func __init__(msg) {
    Exception.__init__(this, msg)
  }
}

class Argument {
  func __init__(args,
                dest,
                nargs,
                type_arg,
                choices,
                required,
                default_value,
                help) {
    this.args = args
    this.dest = dest
    this.nargs = nargs
    this.type_arg = type_arg
    this.choices = choices
    this.required = required
    this.default_value = default_value
    this.help = help
  }

  func match(arg) {
    if this.args is array {
      for a in this.args {
        if arg.find(a) == 0 {
          return true
        }
      }
    } else {
      if this.args is string {
        if arg.find(this.args) == 0 {
          return true
        }
      }
    }

    return false
  }

  func args() {
    return this.args
  }

  func dest() {
    return this.dest
  }

  func nargs() {
    return this.nargs
  }

  func type_arg() {
    return this.type_arg
  }

  func choices() {
    return this.choices
  }

  func default_value() {
    return this.default_value
  }

  func help() {
    return this.help
  }
}

class ArgumentParser {
  func __init__() {
    this.arguments_list = []
    this.vars = {}
    this.others = []
  }

  func add_argument(args,
                    dest,
                    nargs = 1,
                    type_arg = string,
                    choices = [],
                    required = false,
                    default_value = null,
                    help = "") {
      argument = Argument(args = args,
                          dest = dest,
                          nargs = nargs,
                          type_arg = type_arg,
                          choices = choices,
                          required = required,
                          default_value = default_value,
                          help = help)

    this.arguments_list.append(argument)
  }

  func parser_str_arg(arg) {
    pos_eq = arg.find("=")

    if (!pos_eq) {
      throw InvalidArgumentException("'=' not found on argument: ", arg)
    }

    arg_value = arg.split("=")
    return arg_value[1]
  }

  func process_arg(line_arg) {
    for argument in this.arguments_list {
      if argument.match(line_args) {
        switch type(argument.type_arg()) {
          case bool {
            this.vars[argument.dest()] = true
          }

          case string {
            arg_value = parser_str_arg(line_arg)
            if not arg_value in argument.choices() {
              throw InvalidArgumentException("'", arg_value, "'", "is not ",
                  "on choices of argument: ", line_arg)
            }

            this.vars[argument.dest()] = arg_value
          }

          default {
            throw InvalidArgumentException("not valid argument type")
          }
        }
      } else {
        this.others.append(line_args)
      }
    }
  }

  func fill_vars() {
    for argument in arguments_list {
      this.vars += {argument.dest(): null}
    }
  }

  func process(line_args) {
    for line_arg in line_args {
      process_arg(line_arg)
    }

    for argument in this.arguments_list {
      # check if some required argument was not filled
      if argument.required() {
        throw InvalidArgumentException(argument.dest(), " is required")
      } else {
        this.vars[argument.dest()] = argument.default_value()
      }
    }
  }

  func vars() {
    return this.vars
  }
}
