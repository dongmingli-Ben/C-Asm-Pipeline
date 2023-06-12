`include "config.v"

module HazardControl(
    input[31:0] instr,
    input[31:0] nextInstr,
    input[4:0] WriteReg,
    input RegWrite,
    input MemtoReg,
    input[4:0] prevWriteReg,
    input prevRegWrite,
    input prevMemtoReg,
    input stall,
    input[31:0] StallCount,
    output StallOut,
    output[31:0] StallCountOut
);

reg stall_reg;
reg[31:0] count;
wire forward_stall, lw_stall;
// wire next_use_rs, next_use_rt;

ForwardDataHazardModule forward_hazard_module(
    nextInstr[25:21], nextInstr[20:16], WriteReg, RegWrite, 
    prevWriteReg, prevRegWrite, forward_stall);
lwDataHazardModule lw_hazard_module(
    nextInstr[25:21], nextInstr[20:16], WriteReg, MemtoReg, 
    prevWriteReg, prevMemtoReg, lw_stall);

always @(*) begin
    case (instr[31:26])
        6'b000000:
            begin
                case (instr[5:0])
                    6'b001000: // jr
                        begin
                            stall_reg = 1;
                            count = 0;
                        end
                    default: 
                        begin
                            stall_reg = forward_stall || lw_stall;
                            count = 0;
                        end
                endcase
            end
        6'b000100: // beq
            begin 
                stall_reg = 1;
                count = 2;
            end
        6'b000101: // bne
            begin 
                stall_reg = 1;
                count = 2;
            end
        6'b000010: // j
            begin 
                stall_reg = 1;
                count = 0;
            end
        6'b000011: // jal
            begin 
                stall_reg = 1;
                count = 0;
            end
        `STALL_OPCODE: // preserve the input values
            begin 
                if (stall) begin // preserve the input values, if it is stalling
                    stall_reg = stall;
                    count = StallCount;
                end else begin  // detect data hazard between EX and IF
                    stall_reg = forward_stall || lw_stall;
                    count = 0;
                end
            end
        default: // preserve the input values
            begin 
                stall_reg = forward_stall || lw_stall;
                count = 0;
            end
    endcase
end

assign StallOut = stall_reg;
assign StallCountOut = count;

endmodule


module lwDataHazardModule(
    input[4:0] next_rs,
    input[4:0] next_rt,
    input[4:0] WriteReg,
    input MemtoReg,
    input[4:0] prevWriteReg,
    input prevMemtoReg,
    output stall
);

reg stall_reg;

always @(*) begin
    if (MemtoReg && (WriteReg == next_rs || WriteReg == next_rt)) begin
        stall_reg = 1;
    end else if (prevMemtoReg && (prevWriteReg == next_rs || prevWriteReg == next_rt)) begin
        stall_reg = 1;
    end else begin
        stall_reg = 0;
    end
end

assign stall = stall_reg;

endmodule


module ForwardDataHazardModule(
    input[4:0] next_rs,
    input[4:0] next_rt,
    input[4:0] WriteReg,
    input RegWrite,
    input[4:0] prevWriteReg,
    input prevRegWrite,
    output stall
);

reg stall_reg;

always @(*) begin
    if (RegWrite && (WriteReg == next_rs || WriteReg == next_rt)) begin
        stall_reg = 1;
    end else if (prevRegWrite && (prevWriteReg == next_rs || prevWriteReg == next_rt)) begin
        stall_reg = 1;
    end else begin
        stall_reg = 0;
    end
end

assign stall = stall_reg;

endmodule