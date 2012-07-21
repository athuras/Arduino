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
    puts port.read
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
      mode['u'], mode['s'], mode['l'], mode['p'] = 'A', 'B', 'C', 'D';
      text = text.split(' ')
      ins = ""
      begin
        ins.concat( text[0] )
        ins.concat( text[1] )
        if mode.has_key?(text[2])
          ins.concat( mode[text[2]] )
        else
          ins.concat("0")
        end
      rescue 
        @instruction = nil
        puts 'error formatting strings'
      end
      (length - ins.length).times do |e|
        ins.concat('0');
      end
      @instruction = ins
      return self
    end
  end
     
end
