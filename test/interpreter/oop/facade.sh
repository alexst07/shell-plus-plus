# simple class with same name of attributes
# --output:start
# Ouch!
# Beep beep!
# Loading..
# Ready to be used!
# Bup bup bup buzzzz!
# Haaah!
# Zzzzz
# --output:end

class Computer {
  func getElectricShock() {
    echo "Ouch!"
  }

  func makeSound() {
    echo "Beep beep!"
  }

  func showLoadingScreen() {
    echo "Loading.."
  }

  func bam() {
    echo "Ready to be used!"
  }

  func closeEverything() {
    echo "Bup bup bup buzzzz!"
  }

  func sooth() {
    echo "Zzzzz"
  }

  func pullCurrent() {
      echo "Haaah!"
  }
}

class ComputerFacade {
  func __init__(computer) {
    this.computer = computer
  }

  func turnOn() {
    this.computer.getElectricShock();
    this.computer.makeSound();
    this.computer.showLoadingScreen();
    this.computer.bam();
  }

  func turnOff() {
    this.computer.closeEverything();
    this.computer.pullCurrent();
    this.computer.sooth();
  }
}

computer = ComputerFacade(Computer())
computer.turnOn()
computer.turnOff()
