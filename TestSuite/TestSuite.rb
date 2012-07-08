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
  def init(port = '/dev/ttyUSB0')
    port_str = port
    baud_rate = 9600
    data_bits = 8
    stop_bits = 1
    parity = SerialPort::NONE

    sPort = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)
    return
  end
  
  def send(command, wait=5)
    cmd = Instruction.new(command)
    puts "#{command} :: #{cmd.inspect}"
    sPort.write(cmd)
    status = Timeout::timeout(wait){
      while true do 
        printf("%c", sPort.getc)
      end
    };
    return
  end

  class Instruction
    attr_accessor :instruction
    def initialize(text)
      #instruction definitions
      mode{}; mode['u'], mode['s'] = 'UNLOCK_STRING', 'STATUS_STRING';
      text = text.split(' ')
      begin
        instruction = ""
        instruction += mode[text[0]]
        instruction += format_address( text[1], text[2] )
      rescue 
        @instruction = nil
        puts 'error formatting strings'
      end
      @instruction = instruction
      return self
    end
    def format_address(*params)
      out = ""
      # do somthing
      params.each do |p|
        out += ';'; out += p;
      end
      return out
    end
  end
     
end
