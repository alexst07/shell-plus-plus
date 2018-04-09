__all__ = [
  "error",
  "warning",
  "info",
  "success"
]

colors_map = {
  "black": "\e[30m",
  "red": "\e[31m",
  "green": "\e[32m",
  "yellow": "\e[33m",
  "blue": "\e[34m",
  "magenta": "\e[35m",
  "cyan": "\e[36m",
  "lgray": "\e[37m",
  "dgray": "\e[90m",
  "lred": "\e[91m",
  "lgreen": "\e[92m",
  "lyellow": "\e[93m",
  "lblue": "\e[94m",
  "lmagenta": "\e[95m",
  "lcyan": "\e[96m",
  "white": "\e[97m"
}

back_colors_map = {
  "back_black": "\e[30m",
  "back_red": "\e[30m",
  "back_green": "\e[30m",
  "back_yellow": "\e[30m",
  "back_blue": "\e[30m",
  "back_magenta": "\e[30m",
  "back_cyan": "\e[30m",
  "back_lgray": "\e[30m",
  "back_lgray": "\e[30m",
  "back_lred": "\e[30m",
  "back_lgreen": "\e[30m",
  "back_lyellow	": "\e[30m",
  "back_lblue": "\e[30m",
  "back_lmagenta": "\e[30m",
  "back_lcyan": "\e[30m"
}

styles_map = {
  "bold": ("\e[1m", "\e[21m"),
  "dim": ("\e[2m", "\e[22m"),
  "underlined": ("\e[4m", "\e[24m"),
  "blink": ("\e[5m", "\e[25m"),
  "reverse": ("\e[7m", "\e[27m"),
  "hidden": ("\e[8m", "\e[28m")
}

func error(args...) {
  print_err("\e[1m\e[31m[ERROR] \e[0m\e[31m", ...args, "\e[0m")
}

func warning(args...) {
  print("\e[1m\e[35m[WARNING] \e[0m\e[35m", ...args, "\e[0m")
}

func info(args...) {
  print("\e[1m\e[94m[INFO] \e[0m\e[94m", ...args, "\e[0m")
}

func success(args...) {
  print("\e[1m\e[92m[SUCCESS] \e[0m\e[92m", ...args, "\e[0m")
}

func join_args(args...) {
  str = [...args].join("")
  return str
}

func insert_var(var_name, obj) {
  str_code = var_name + "=" + "obj"
  eval(str_code)
}

for style_name, style in colors_map {
  fn = func(args...) {
    return style + join_args(...args) + "\e[39m"
  }

  insert_var(style_name, fn)
  __all__.append(style_name)
}

for style_name, style in back_colors_map {
  fn = func(args...) {
    return style + join_args(...args) + "\e[49m"
  }

  insert_var(style_name, fn)
  __all__.append(style_name)
}

for style_name, style in styles_map {
  fn = func(args...) {
    return style[0] + join_args(...args) + style[0]
  }

  insert_var(style_name, fn)
  __all__.append(style_name)
}
