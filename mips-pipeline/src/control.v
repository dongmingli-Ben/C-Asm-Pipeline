// control signals
module ControlUnit(
    input[5:0] opcode,
    input[5:0] funct,
    output RegWrite,
    output MemtoReg,
    output MemWrite,
    output Branch,
    output[3:0] ALUControl,
    output ALUSrc,
    output RegDst,
    output ImmSrc,
    output ALUShiftSrcA,
    output ALUShiftSrcB,
    output ImmExtSrc,
    output BranchEqual,
    output Jump,
    output JumpReg,
    output RegDstJal,
    output RegRD1OutSrc
);

ALUControlModule aluctr_module(opcode, funct, ALUControl);
RegWriteModule regwrite_module(opcode, funct, RegWrite);
MemtoRegModule memtoreg_module(opcode, funct, MemtoReg);
MemWriteModule memwrite_module(opcode, funct, MemWrite);
BranchModule branch_module(opcode, funct, Branch);
ALUSrcModule alusrc_module(opcode, funct, ALUSrc);
RegDstModule regdst_module(opcode, funct, RegDst);
ImmExtSrcModule immextsrc_module(opcode, funct, ImmExtSrc);
ImmSrcModule immsrc_module(opcode, funct, ImmSrc);
ALUShiftSrcAModule alushiftsrc_a_module(opcode, funct, ALUShiftSrcA);
ALUShiftSrcBModule alushiftsrc_b_module(opcode, funct, ALUShiftSrcB);
BranchEqualModule branch_eq_module(opcode, funct, BranchEqual);
// jumps
JumpModule jump_module(opcode, funct, Jump);
JumpRegModule jumpreg_module(opcode, funct, JumpReg);
RegDstJalModule regdst_jal_module(opcode, funct, RegDstJal);
RegRD1OutSrcModule rd1_out_src_module(opcode, funct, RegRD1OutSrc);

endmodule

module RegWriteModule(input[5:0] opcode, input[5:0] funct, output RegWrite);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin 
            case (funct)
                6'b001000: signal = 0; // jr
                default: signal = 1; // other r type
            endcase
        end
        6'b101011: signal = 0; // sw
        6'b000100: signal = 0; // beq
        6'b000101: signal = 0; // bne
        6'b000010: signal = 0; // j
        6'b000011: signal = 1; // jal
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 1; // other instructions
    endcase
end
assign RegWrite = signal;

endmodule

module MemtoRegModule(input[5:0] opcode, input[5:0] funct, output MemtoReg);

reg signal;
always @(*) begin
    case (opcode)
        6'b100011: signal = 1; // lw
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign MemtoReg = signal;

endmodule

module MemWriteModule(input[5:0] opcode, input[5:0] funct, output MemWrite);

reg signal;
always @(*) begin
    case (opcode)
        6'b101011: signal = 1; // sw
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign MemWrite = signal;

endmodule

module BranchModule(input[5:0] opcode, input[5:0] funct, output Branch);

reg signal;
always @(*) begin
    case (opcode)
        6'b000100: signal = 1; // beq
        6'b000101: signal = 1; // bne
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign Branch = signal;

endmodule

module BranchEqualModule(input[5:0] opcode, input[5:0] funct, output BranchEqual);

reg signal;
always @(*) begin
    case (opcode)
        6'b000100: signal = 1; // beq
        6'b000101: signal = 0; // bne
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign BranchEqual = signal;

endmodule

module ALUSrcModule(input[5:0] opcode, input[5:0] funct, output ALUSrc);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b000000: signal = 1; // sll
                6'b000010: signal = 1; // srl
                6'b000011: signal = 1; // sra
                default: signal = 0; // other r-type instructions
            endcase
        end
        6'b000100: signal = 0; // beq
        6'b000101: signal = 0; // bne
        6'b000011: signal = 0; // jal
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 1; // i-type instructions
    endcase
end
assign ALUSrc = signal;

endmodule

module RegDstModule(input[5:0] opcode, input[5:0] funct, output RegDst);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: signal = 1; // r-type instructions
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // i-type and j-type instructions
    endcase
end
assign RegDst = signal;

endmodule

module ImmSrcModule(input[5:0] opcode, input[5:0] funct, output ImmSrc);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b000000: signal = 1; // sll
                6'b000010: signal = 1; // srl
                6'b000011: signal = 1; // sra
                default: signal = 0; // other r-type instructions
            endcase
        end
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // i-type and j-type instructions
    endcase
end
assign ImmSrc = signal;

endmodule

module ImmExtSrcModule(input[5:0] opcode, input[5:0] funct, output ImmExtSrc);

reg signal;
always @(*) begin
    case (opcode)
        6'b001100: signal = 1; // andi
        6'b001101: signal = 1; // ori
        6'b001110: signal = 1; // xori
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign ImmExtSrc = signal;

endmodule

module ALUShiftSrcAModule(input[5:0] opcode, input[5:0] funct, output ALUShiftSrcA);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b000000: signal = 1; // sll
                6'b000010: signal = 1; // srl
                6'b000011: signal = 1; // sra
                6'b000100: signal = 1; // sllv
                6'b000110: signal = 1; // srlv
                6'b000111: signal = 1; // srav
                default: signal = 0; // other r-type instructions
            endcase
        end
        6'b000011: signal = 0; // jal
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // i-type and j-type instructions
    endcase
end
assign ALUShiftSrcA = signal;

endmodule

module ALUShiftSrcBModule(input[5:0] opcode, input[5:0] funct, output ALUShiftSrcB);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b000100: signal = 1; // sllv
                6'b000110: signal = 1; // srlv
                6'b000111: signal = 1; // srav
                default: signal = 0; // other r-type instructions
            endcase
        end
        6'b000011: signal = 1; // jal
        `STALL_OPCODE: signal = 0; // nop
        default: signal = 0; // i-type and j-type instructions
    endcase
end
assign ALUShiftSrcB = signal;

endmodule

module JumpModule(input[5:0] opcode, input[5:0] funct, output Jump);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b001000: signal = 1; // jr
                default: signal = 0; // other r-type instructions
            endcase
        end 
        6'b000010: signal = 1; // j
        6'b000011: signal = 1; // jal
        `NOP_INST: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign Jump = signal;

endmodule

module JumpRegModule(input[5:0] opcode, input[5:0] funct, output JumpReg);

reg signal;
always @(*) begin
    case (opcode)
        6'b000000: begin
            case (funct)
                6'b001000: signal = 1; // jr
                default: signal = 0; // other r-type instructions
            endcase
        end
        `NOP_INST: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign JumpReg = signal;

endmodule

module RegDstJalModule(input[5:0] opcode, input[5:0] funct, output RegDstJal);

reg signal;
always @(*) begin
    case (opcode)
        6'b000011: signal = 1; // jal
        `NOP_INST: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign RegDstJal = signal;

endmodule

module RegRD1OutSrcModule(input[5:0] opcode, input[5:0] funct, output RegRD1OutSrc);

reg signal;
always @(*) begin
    case (opcode)
        6'b000011: signal = 1; // jal
        `NOP_INST: signal = 0; // nop
        default: signal = 0; // other instructions
    endcase
end
assign RegRD1OutSrc = signal;

endmodule


module ALUControlModule(input[5:0] opcode, input[5:0] funct, output reg[3:0] aluctr);
    wire[3:0] add_ctr, sub_ctr, and_ctr, nor_ctr, or_ctr, sll_ctr, srl_ctr, 
             sra_ctr, slt_ctr, sltu_ctr, r_type_ctr, xor_ctr;
    assign add_ctr = 4'b0010;
    assign sub_ctr = 4'b0110;
    assign and_ctr = 4'b0000;
    assign nor_ctr = 4'b0011;
    assign or_ctr = 4'b0001;
    assign sll_ctr = 4'b1000;
    assign srl_ctr = 4'b1001;
    assign sra_ctr = 4'b1010;
    assign slt_ctr = 4'b0111;
    assign sltu_ctr = 4'b1011;
    assign xor_ctr = 4'b1100;
    RTypeALUCtrModule alu_m(funct, r_type_ctr);
    always @(*) begin
        case (opcode)
            6'b000000: aluctr = r_type_ctr;
            6'b100011: aluctr = add_ctr;
            6'b101011: aluctr = add_ctr;
            6'b001000: aluctr = add_ctr;
            6'b001001: aluctr = add_ctr;
            6'b001110: aluctr = xor_ctr;
            6'b001100: aluctr = and_ctr;
            6'b001101: aluctr = or_ctr;
            6'b000100: aluctr = sub_ctr;
            6'b000101: aluctr = sub_ctr;
            6'b000010: aluctr = add_ctr;
            6'b001000: aluctr = add_ctr;
            6'b000011: aluctr = or_ctr; // jal
            `STALL_OPCODE: aluctr = add_ctr; // nop
            default: begin
                aluctr = 4'b1111;
                // #10
                // $display("unsupported opcode: %6b", opcode);
            end
        endcase
    end
endmodule


module RTypeALUCtrModule(input[5:0] funct, output reg[3:0] aluctr);
    wire[3:0] add_ctr, sub_ctr, and_ctr, nor_ctr, or_ctr, sll_ctr, 
        srl_ctr, sra_ctr, slt_ctr, sltu_ctr, xor_ctr;
    assign add_ctr = 4'b0010;
    assign sub_ctr = 4'b0110;
    assign and_ctr = 4'b0000;
    assign nor_ctr = 4'b0011;
    assign or_ctr = 4'b0001;
    assign sll_ctr = 4'b1000;
    assign srl_ctr = 4'b1001;
    assign sra_ctr = 4'b1010;
    assign slt_ctr = 4'b0111;
    assign sltu_ctr = 4'b1011;
    assign xor_ctr = 4'b1100;
    always @(*) begin
        case (funct)
            6'b100000: aluctr = add_ctr;
            6'b100001: aluctr = add_ctr;
            6'b100110: aluctr = xor_ctr;
            6'b100010: aluctr = sub_ctr;
            6'b100011: aluctr = sub_ctr;
            6'b100100: aluctr = and_ctr;
            6'b100111: aluctr = nor_ctr;
            6'b100101: aluctr = or_ctr;
            6'b000000: aluctr = sll_ctr;
            6'b000100: aluctr = sll_ctr;
            6'b000010: aluctr = srl_ctr;
            6'b000110: aluctr = srl_ctr;
            6'b000011: aluctr = sra_ctr;
            6'b000111: aluctr = sra_ctr;
            6'b101010: aluctr = slt_ctr;
            default: begin
                aluctr = 4'b1111;
                // #10
                // $display("unsupported funct: %6b", funct);
            end
        endcase
        // #10
        // $display("rtype funct: %4b", funct);
        // $display("rtype aluctr: %4b", aluctr);
    end
endmodule


module BranchControl(
    input Branch,
    input Zero,
    input BranchEqual,
    output PCSrcBranch
);

reg signal;
always @(*) begin
    if (Branch) begin
        if ((BranchEqual && Zero) || (~BranchEqual && ~Zero))
            signal = 1;
        else signal = 0;
    end else signal = 0;
end
assign PCSrcBranch = signal;

endmodule