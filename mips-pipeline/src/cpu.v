`include "config.v"
// CLK: input clock signal
module CPU
(
    //input
      input CLK
);

// stage 1
reg[31:0] PCF;
wire[31:0] PC, InstrF, PCPlus4F;
// stage 2
reg[31:0] InstrD, PCPlus4D;
wire[4:0] RegA1, RegA2, RegA3;
wire[31:0] RegRD1D, RegRD2D, SignImmExtD, UnsignImmExtD, SAImmExtD, ImmExtD, ImmD,
  PCJumpLabelD, PCJumpRegD, PCJumpD, RegRD1OutD;
wire[4:0] WriteRegD, WriteRegDPrior;
// this is a special variable which connects to 
// a variable from ID and one from MEM.
// NOTE: when it is JUMP, the PCBranchM should be useless (because of stall in beq/bne)
// it is safe to directly connect PCJumpD to PCBranchAddr
wire[31:0] PCBranchAddr;   
wire[4:0] RtD, RdD;
// stage 3
reg[31:0] SignImmE, PCPlus4E, RegRD1E, RegRD2E;
wire[31:0] WriteDataE, ALUOutE, PCBranchE, SrcAE, SrcBE, SrcBEPrior;
reg[4:0] WriteRegE;
wire[2:0] ALUFlagE;
reg[4:0] RegRA = 31;
// stage 4
reg ZeroM;
reg[31:0] ALUOutM, WriteDataM, PCBranchM;
reg[4:0] WriteRegM;
wire[31:0] ReadDataM;
// stage 5
reg[31:0] ALUOutW, ReadDataW;
reg[4:0] WriteRegW;
wire[31:0] ResultW;

// control signals
wire RegWriteD, MemtoRegD, MemWriteD, BranchD, ALUSrcD, RegDstD, 
  ImmSrcD, ImmExtSrcD, ALUShiftSrcAD, ALUShiftSrcBD, BranchEqualD,
  JumpD, JumpRegD, RegRD1OutSrcD, RegDstJalD;
wire[3:0] ALUControlD;
reg RegWriteE, MemtoRegE, MemWriteE, BranchE, ALUSrcE, 
  ALUShiftSrcAE, ALUShiftSrcBE, BranchEqualE;
reg[3:0] ALUControlE;
reg RegWriteM, MemtoRegM, MemWriteM, BranchM, BranchEqualM;
reg RegWriteW, MemtoRegW;
wire PCSrcBranchM, PCSrc;

// special signals (such as for hazard)
reg stall;
reg InstrEnd;
reg[31:0] StallCount = 0;
wire StallOut;
wire[31:0] StallCountOut;

// modules
InstructionMemory inst_mem(PCF, InstrF);
RegisterFile reg_file(CLK, RegA1, RegA2, RegA3, RegWriteW, ResultW, RegRD1D, RegRD2D);
SignExtension sign_ext(InstrD[15:0], SignImmExtD);
UnsignExtension unsign_ext(InstrD[15:0], UnsignImmExtD);
ExtensionSA sign_ext_sa(InstrD[10:6], SAImmExtD);
ALU alu_module(SrcAE, SrcBE, ALUControlE, ALUOutE, ALUFlagE);
DataMemory data_mem(CLK, ALUOutM, WriteDataM, MemWriteM, ReadDataM);
HazardControl hazard_control(
  InstrD, InstrF, WriteRegD, RegWriteD, MemtoRegD, 
  WriteRegE, RegWriteE, MemtoRegE,
  stall, StallCount, StallOut, StallCountOut);

// control module
ControlUnit control_unit(InstrD[31:26], InstrD[5:0],
  RegWriteD, MemtoRegD, MemWriteD, BranchD, ALUControlD, 
  ALUSrcD, RegDstD, ImmSrcD, ALUShiftSrcAD, ALUShiftSrcBD,
  ImmExtSrcD, BranchEqualD, JumpD, JumpRegD, RegDstJalD,
  RegRD1OutSrcD);
BranchControl branch_control(BranchM, ZeroM, BranchEqualM, PCSrcBranchM);

// signal stall before the posedge
always @(negedge CLK) begin
  stall <= StallOut;
  StallCount <= StallCountOut;
end

// stage registers
always @(posedge CLK) begin
  if (stall) begin
    PCF <= PC - 4;  // so that PC+4 remains unchanged
    // IF/ID
    InstrD <= `NOP_INST; // insert nop
    // #1
    if (~InstrEnd) begin
      if (StallCount == 0) begin
        stall <= 0;
      end else begin
        StallCount = StallCount - 1;
      end
    end
  end else begin
    PCF <= PC;
    // IF/ID
    InstrD <= InstrF;
  end
  // IF/ID
  PCPlus4D <= PCPlus4F;
  // ID/EX
  RegWriteE <= RegWriteD;
  MemtoRegE <= MemtoRegD;
  MemWriteE <= MemWriteD;
  BranchE <= BranchD;
  ALUControlE <= ALUControlD;
  ALUSrcE <= ALUSrcD;
  // RegDstE <= RegDstD;
  RegRD1E <= RegRD1OutD;
  RegRD2E <= RegRD2D;
  // RtE <= InstrD[20:16];
  // RdE <= InstrD[15:11];
  SignImmE <= ImmD;
  PCPlus4E <= PCPlus4D;
  ALUShiftSrcAE <= ALUShiftSrcAD;
  ALUShiftSrcBE <= ALUShiftSrcBD;
  BranchEqualE <= BranchEqualD;
  WriteRegE <= WriteRegD;
  // RegDstJalE <= RegDstJalD;
  // EX/MEM
  RegWriteM <= RegWriteE;
  MemtoRegM <= MemtoRegE;
  MemWriteM <= MemWriteE;
  BranchM <= BranchE;
  ZeroM <= ALUFlagE[2];
  ALUOutM <= ALUOutE;
  WriteDataM <= WriteDataE;
  WriteRegM <= WriteRegE;
  PCBranchM <= PCBranchE;
  BranchEqualM <= BranchEqualE;
  // MEM/WB
  RegWriteW <= RegWriteM;
  MemtoRegW <= MemtoRegM;
  ALUOutW <= ALUOutM;
  ReadDataW <= ReadDataM;
  WriteRegW <= WriteRegM;

  `ifdef DEBUG
  // for debug
  $stop;
  // #1;
  $display("status:");
  $display("Stall: %1b", stall);
  $display("StallCount: %d", StallCount);
  $display("StallOut: %1b", StallOut);
  $display("StallCountOut: %d", StallCountOut);
  $display("Is end: %1b", InstrEnd);
  $display("==============IF stage==============");
  $display("PC: %d", PC);
  $display("PCF: %d", PCF);
  $display("InstrF: %32b", InstrF);
  $display("PCPlus4F: %d", PCPlus4F);
  $display("==============ID stage==============");
  $display("InstrD: %32b", InstrD);
  $display("PCPlus4D: %d", PCPlus4D);
  $display("RegA1: %d", RegA1);
  $display("RegA2: %d", RegA2);
  $display("RegA3: %d", RegA3);
  $display("RegRD1D: %d", RegRD1D);
  $display("RegRD1OutD: %d", RegRD1OutD);
  $display("RegRD2D: %d", RegRD2D);
  $display("PCJumpRegD: %d", PCJumpRegD);
  $display("PCJumpLabelD: %d", PCJumpLabelD);
  $display("SignImmExtD: %d", SignImmExtD);
  $display("UnsignImmExtD: %d", UnsignImmExtD);
  $display("SAImmExtD: %d", SAImmExtD);
  $display("ImmD: %d", ImmD);
  $display("PCBranchAddr: %d", PCBranchAddr);
  $display("RtD: %d", RtD);
  $display("RdD: %d", RdD);
  $display("WriteRegDPrior: %d", WriteRegDPrior);
  $display("WriteRegD: %d", WriteRegD);
  $display("--> control:");
  $display("ALUControlD: %3b", ALUControlD);
  $display("RegWriteD: %1b", RegWriteD);
  $display("MemtoRegD: %1b", MemtoRegD);
  $display("MemWriteD: %1b", MemWriteD);
  $display("BranchD: %1b", BranchD);
  $display("BranchEqualD: %1b", BranchEqualD);
  $display("ALUSrcD: %1b", ALUSrcD);
  $display("RegDstD: %1b", RegDstD);
  $display("ImmExtSrcD: %1b", ImmExtSrcD);
  $display("ImmSrcD: %1b", ImmSrcD);
  $display("ALUShiftSrcAD: %1b", ALUShiftSrcAD);
  $display("ALUShiftSrcBD: %1b", ALUShiftSrcBD);
  $display("JumpD: %1b", JumpD);
  $display("JumpRegD: %1b", JumpRegD);
  $display("RegRD1OutSrcD: %1b", RegRD1OutSrcD);
  $display("RegDstJalD: %1b", RegDstJalD);
  $display("==============EX stage==============");
  $display("SrcAE: %d", SrcAE);
  $display("SrcBE: %d", SrcBE);
  $display("SrcBEPrior: %d", SrcBEPrior);
  $display("SignImmE: %d", SignImmE);
  $display("WriteDataE: %d", WriteDataE);
  $display("PCPlus4E: %d", PCPlus4E);
  $display("RegRD1E: %d", RegRD1E);
  $display("RegRD2E: %d", RegRD2E);
  $display("ALUOutE: %d", ALUOutE);
  $display("PCBranchE: %d", PCBranchE);
  // $display("WriteRegEPrior: %d", WriteRegEPrior);
  $display("WriteRegE: %d", WriteRegE);
  $display("ALUFlagE: %3b", ALUFlagE);
  $display("--> control:");
  $display("ALUControlE: %3b", ALUControlE);
  $display("RegWriteE: %1b", RegWriteE);
  $display("MemtoRegE: %1b", MemtoRegE);
  $display("MemWriteE: %1b", MemWriteE);
  $display("BranchE: %1b", BranchE);
  $display("BranchEqualE: %1b", BranchEqualE);
  $display("ALUSrcE: %1b", ALUSrcE);
  // $display("RegDstE: %1b", RegDstE);
  $display("ALUShiftSrcAE: %1b", ALUShiftSrcAE);
  $display("ALUShiftSrcBE: %1b", ALUShiftSrcBE);
  // $display("RegDstJalE: %1b", RegDstJalE);
  $display("==============MEM stage==============");
  $display("ZeroM: %b", ZeroM);
  $display("ALUOutM: %d", ALUOutM);
  $display("WriteDataM: %d", WriteDataM);
  $display("PCBranchM: %d", PCBranchM);
  $display("WriteRegM: %d", WriteRegM);
  $display("ReadDataM: %d", ReadDataM);
  $display("--> control:");
  $display("RegWriteM: %d", RegWriteM);
  $display("MemtoRegM: %d", MemtoRegM);
  $display("MemWriteM: %d", MemWriteM);
  $display("BranchM: %d", BranchM);
  $display("BranchEqualM: %1b", BranchEqualM);
  $display("PCSrcBranchM: %d", PCSrcBranchM);
  $display("==============WB stage==============");
  $display("ALUOutW: %d", ALUOutW);
  $display("ReadDataW: %d", ReadDataW);
  $display("WriteRegW: %d", WriteRegW);
  $display("ResultW: %d", ResultW);
  $display("--> control:");
  $display("RegWriteW: %d", RegWriteW);
  $display("MemtoRegW: %d", MemtoRegW);
  $display("==============Special==============");
  $display("PCSrc: %d", PCSrc);
  `endif
end

// other combinatorial modules
assign PCPlus4F = PCF + 4;
assign PCBranchE = PCPlus4E + (SignImmE << 2);
assign RegA1 = InstrD[25:21];
assign RegA2 = InstrD[20:16];
assign RegA3 = WriteRegW;
assign WriteDataE = RegRD2E;
assign PCJumpLabelD = {PCF[31:28], InstrD[25:0] << 2};
assign PCJumpRegD = RegRD1D;
assign RtD = InstrD[20:16];
assign RdD = InstrD[15:11];

// multiplexors
// IMPORTANT
// +4 because in current implementation of stall,
// PCF <= PC - 4 to keep the PCF unchanged in stall.
// Therefore, a hardcoded +4 is used to offset the last stall in branch
Multiplex2to1b32 pc_multiplex(PCPlus4F, PCBranchAddr+4, PC, PCSrc);
Multiplex2to1b32 ext_multiplex(SignImmExtD, UnsignImmExtD, ImmExtD, ImmExtSrcD);
Multiplex2to1b32 sa_multiplex(ImmExtD, SAImmExtD, ImmD, ImmSrcD);
Multiplex2to1b5 reg_dst_prior_multiplex(RtD, RdD, WriteRegDPrior, RegDstD);
Multiplex2to1b5 writereg_prior_multiplex(WriteRegDPrior, RegRA, WriteRegD, RegDstJalD);
Multiplex2to1b32 alu_shift_src_a_multiplex(RegRD1E, RegRD2E, SrcAE, ALUShiftSrcAE);
Multiplex2to1b32 alu_shift_src_b_multiplex(RegRD2E, RegRD1E, SrcBEPrior, ALUShiftSrcBE);
Multiplex2to1b32 alu_src_multiplex(SrcBEPrior, SignImmE, SrcBE, ALUSrcE);
Multiplex2to1b32 write_multiplex(ALUOutW, ReadDataW, ResultW, MemtoRegW);
Multiplex2to1b1 pcsrc_multiplex(PCSrcBranchM, 1'b1, PCSrc, JumpD);
Multiplex2to1b32 pcjump_multiplex(PCJumpLabelD, PCJumpRegD, PCJumpD, JumpRegD);
Multiplex2to1b32 pcbranch_addr_multiplex(PCBranchM, PCJumpD, PCBranchAddr, JumpD);
Multiplex2to1b32 reg_rd1_out_multiplex(RegRD1D, PCF, RegRD1OutD, RegRD1OutSrcD);



// set up special signals
initial begin
  stall = 0;
  InstrEnd = 0;
end

// set up initial values
initial begin
  PCF = 0;
end

// handles instruction end
integer f;
integer remain_instr_count = 3, data_i;
always @(negedge CLK) begin
  if (InstrEnd == 0 && InstrD == `END_INST) begin
    InstrEnd <= 1;
    stall <= 1;
    `ifdef DEBUG
    $display("encounter end instruction");
    `endif
  end
  if (remain_instr_count > 0 && InstrEnd) begin
    remain_instr_count <= remain_instr_count - 1;
  end
  if (InstrEnd && remain_instr_count == 0) begin
    f = $fopen("data.bin", "w");
    for (data_i = 0; data_i < `N_DATA; data_i = data_i + 1) begin
      $fwrite(f, "%32b\n", data_mem.memory[data_i]);
    end
    $fclose(f);
    $finish;
  end
end


endmodule