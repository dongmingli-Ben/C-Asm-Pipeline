`include "config.v"

// multiplexor
// 32 bits
module Multiplex2to1b32(input[31:0] src1, input[31:0] src2, output[31:0] out, input sgl);

reg[31:0] result;
always @(*) begin
    if (sgl) result = src2;
    else result = src1;
end
assign out = result;

endmodule

// 5 bits
module Multiplex2to1b5(input[4:0] src1, input[4:0] src2, output[4:0] out, input sgl);

reg[4:0] result;
always @(*) begin
    if (sgl) result = src2;
    else result = src1;
end
assign out = result;

endmodule

// 1 bits
module Multiplex2to1b1(input src1, input src2, output out, input sgl);

reg result;
always @(*) begin
    if (sgl) result = src2;
    else result = src1;
end
assign out = result;

endmodule

// signed extension
module SignExtension(input[15:0] src, output[31:0] out);

assign out = { {16{src[15]}}, src[15:0] };

endmodule

// unsigned extension
module UnsignExtension(input[15:0] src, output[31:0] out);

assign out = { 16'h0000, src[15:0] };

endmodule

module ExtensionSA(input[4:0] src, output[31:0] out);

assign out = { {27{1'b0}}, src[4:0] };

endmodule

// reg file
module RegisterFile(
    input CLK, 
    input[4:0] A1,
    input[4:0] A2,
    input[4:0] A3,
    input WriteEnabled,
    input[31:0] WriteData,
    output[31:0] RD1,
    output[31:0] RD2
    );

reg[31:0] registers[0:31];
// initialize values to 0
integer i;
initial begin
    for (i = 0; i < 32; i++) 
        registers[i] = 0;
end

// read
assign RD1 = registers[A1];
assign RD2 = registers[A2];

// write
always @(negedge CLK) begin
    if (WriteEnabled) begin
        registers[A3] <= WriteData;
    end
end

`ifdef DEBUG
// for debug, display register values
integer j;
always @(posedge CLK) begin
    // todo: print register id and value, eight items per row
    #1
    $display("registers:");
    for (i = 0; i < 32; i = i + 8) begin
        $write("%0d:\t", i);
        for (j = 0; j < 8; j = j + 1) begin
            $write("%08x\t", registers[i+j]);
        end
        $write("\n");
    end
end
`endif

endmodule

// RAMs
module InstructionMemory(input[31:0] pc, output[31:0] inst);

reg[31:0] memory[0:`N_INSTR-1];

// set up instruction memory in initialization
integer i;
initial begin
    $readmemb("CPU_instruction.bin", memory);
    `ifdef DEBUG
    for (i = 0; i < `N_INSTR; i = i + 1)
        $display("%b", memory[i]);
    `endif
end

assign inst = memory[pc >> 2];

endmodule

module DataMemory(
    input CLK, 
    input[31:0] addr, 
    input[31:0] WriteData, 
    input WriteEnabled, 
    output[31:0] RD);

reg[31:0] memory[0:`N_DATA-1];

// set up data memory in initialization
integer i;
initial begin
    for (i = 0; i < `N_DATA; i = i + 1) begin
        memory[i] = 0;
    end
end

// read
assign RD = memory[addr >> 2];

// write
always @(negedge CLK) begin
    if (WriteEnabled) begin
        memory[addr >> 2] <= WriteData;
    end
end

`ifdef DEBUG
// for debug, display memory values
integer j;
always @(posedge CLK) begin
    // todo: print register id and value, eight items per row
    #2
    $display("data memory:");
    for (i = 0; i < 80; i = i + 8) begin
        $write("%0d:\t", i << 2);
        for (j = 0; j < 8; j = j + 1) begin
            $write("%08x\t", memory[i+j]);
        end
        $write("\n");
    end
end
`endif

endmodule