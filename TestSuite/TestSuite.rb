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
    return sPort = SerialPort.new(port_str, baud_rate, data_bits, stop_bits, parity)
  end
  
  def send(command, port, wait=3)
    cmd = Instruction.new(command)
    puts "#{command} :: #{cmd.inspect}"
    port.write(cmd)
    status = Timeout::timeout(wait){
      while true do 
        printf("%c", port.getc)
      end
    };
    return
  end

  class Instruction
    attr_accessor :instruction, :length
    def initialize(text, length = 10)
      #instruction definitions
      mode = {}; 
      mode['u'], mode['s'], mode['l'], mode['p'] = 'A', 'B', 'C', 'D';
      text = text.split(' ')
      begin
        instruction = ""
        text.each do |e|
          if mode.has_key?(e)
            instruction.concat(mode[e])
          else 
            instruction.concat(e)
          end
        end
      rescue 
        @instruction = nil
        puts 'error formatting strings'
      end
      (length - instruction.length).times do |e|
        instruction = instruction.concat(0);
      end
      @instruction = instruction
      return self
    end
    def format_address(*params)
      out = ""
      params.each do |p|
        out += p;
      end
      return out
    end
  end
     
end
