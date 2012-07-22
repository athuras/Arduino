#!/usr/bin/env ruby
# Intended to be used with irb to streamline serial testing of microcontroller network
#
# in irb, require TestSuite.rb include TestSuite
# generate a new Instruction message and send it via:
# =>send("u 24 4") # unlock column 24, cell 4

module TestSuite

  require 'rubygems'
  require 'serialport' # use Kernel::require on windows if necessary
  require 'timeout'

  # Serial Port Parameters
  def init(port = '/dev/cu.usbmodem411')
    port_str = port
    baud_rate = 9600
    data_bits = 8
    stop_bits = 1
    parity = SerialPort::NONE
    sPort = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)
    sPort.read_timeout = 0
    return sPort
  end
  
  def io(msg, port)
    s = []
    port.write(msg.instruction)
    s.push(port.read)
    return s
  end

  def send(command, port, wait=2)
    cmd = Instruction.new(command)
    puts "#{command} :: #{cmd.inspect}"
    s = [];
    port.write(cmd.instruction)
    status = Timeout::timeout(wait){
      while true do 
        printf("%c", port.getbyte)
      end
    };
  end

  class Instruction
    attr_accessor :instruction, :length
    def initialize(text, length = 10)
      #instruction definitions Give
      mode = {};
      ins = text.split(' ')
      (length - ins.length).times do |e|
        ins.push(0);
      end
      text = text.map{ |e| e.chr }
      @instruction = text.to_s
      return self
    end
  end
  
end
