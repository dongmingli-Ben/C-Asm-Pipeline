`timescale 1ns/1ps
`default_nettype none

module testbench;

reg clock;

initial begin
    clock = 0;
    forever #10 clock = ~clock;
end

CPU cpu_module(clock);

endmodule