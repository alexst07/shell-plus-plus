# simple class with same name of attributes
# --output:start
# TextField: 80, 24
# ScrollDecorator
# BorderDecorator
# BorderDecorator
# --output:end

interface Widget {
  func draw()
}

class TextField: Widget {
  func __init__(w, h) {
    this.w = w
    this.h = h
  }

  func draw() {
    print("TextField: ", this.w, ", ", this.h)
  }
}

class Decorator: Widget {
  func __init__(wid) {
    this.wid = wid
  }

  func draw() {
    this.wid.draw()
  }
}

class BorderDecorator(Decorator) {
  func __init__(wid) {
    Decorator.__init__(this, wid)
  }

  func draw() {
    Decorator.draw(this)
    print("BorderDecorator")
  }
}

class ScrollDecorator(Decorator) {
  func __init__(wid)
   {
    Decorator.__init__(this, wid)
  }

  func draw() {
    Decorator.draw(this)
    print("ScrollDecorator")
  }
}

wid = BorderDecorator(
        BorderDecorator(
        ScrollDecorator(
        TextField(80, 24))))

wid.draw()
