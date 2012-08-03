# Intended to be used with irb to streamline serial testing of microcontroller network
#
# in irb, require TestSuite.rb include TestSuite
# generate a new Instruction message and send it via:
# =>send("u 24 4") # unlock column 24, cell 4
# This may not work with 'tricked out slave'

module TestSuite

  require 'rubygems'
  require 'serialport' # use Kernel::require on windows if necessary
  require 'timeout'

  # Serial Port Parameters
  def init(port = '/dev/cu.usbmodem411', baud = 9600)
    port_str = port
    baud_rate = baud
    data_bits = 8
    stop_bits = 1
    parity = SerialPort::NONE
    sPort = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)
    sPort.read_timeout = 0
    return sPort
  end

  def io(msg, port) # Sends a message object down the port, and reads the response
    s = []
    port.write(msg.instruction)
    s.push(port.read)
    return s
  end

  def send(port, instr=[0x00,0x00,0x00], n = 10)
    (n - instr.size).times do
      instr.push(0x00)
    end
    instr.each do |e|
      port.write(e)
    end
    listen(port,2)
    return
  end

  def listen(x, t = 1)
    status = Timeout::timeout(t){
      while true do
        puts x.read
      end
    }
    return status
  end

end
