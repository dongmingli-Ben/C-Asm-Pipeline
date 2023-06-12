// flags[2] : zero flag
// flags[1] : negative flag
// flags[0] : overflow flag 

module ALU(input[31:0] in1, input[31:0] in2, input[3:0] aluctr, output[31:0] result, output[2:0] flags);
    wire[31:0] add_r, sub_r, and_r, nor_r, or_r, sll_r, srl_r, sra_r, slt_r, sltu_r, xor_r;
    wire[2:0] add_f, sub_f, and_f, nor_f, or_f, sll_f, srl_f, sra_f, slt_f, sltu_f, xor_f;
    reg[31:0] res_reg;
    reg[2:0] flag_reg;
    add_module add_m(in1, in2, add_r, add_f);
    sub_module sub_m(in1, in2, sub_r, sub_f);
    and_module and_m(in1, in2, and_r, and_f);
    nor_module nor_m(in1, in2, nor_r, nor_f);
    or_module or_m(in1, in2, or_r, or_f);
    sll_module sll_m(in1, in2, sll_r, sll_f);
    srl_module srl_m(in1, in2, srl_r, srl_f);
    sra_module sra_m(in1, in2, sra_r, sra_f);
    slt_module slt_m(in1, in2, slt_r, slt_f);
    sltu_module sltu_m(in1, in2, sltu_r, sltu_f);
    xor_module xor_m(in1, in2, xor_r, xor_f);
    always @(*) begin
        case (aluctr)
            4'b0010: begin res_reg = add_r; flag_reg = add_f; end
            4'b0110: begin res_reg = sub_r; flag_reg = sub_f; end
            4'b0000: begin res_reg = and_r; flag_reg = and_f; end
            4'b0011: begin res_reg = nor_r; flag_reg = nor_f; end
            4'b0001: begin res_reg = or_r; flag_reg = or_f; end
            4'b1000: begin res_reg = sll_r; flag_reg = sll_f; end
            4'b1001: begin res_reg = srl_r; flag_reg = srl_f; end
            4'b1010: begin res_reg = sra_r; flag_reg = sra_f; end
            4'b0111: begin res_reg = slt_r; flag_reg = slt_f; end
            4'b1011: begin res_reg = sltu_r; flag_reg = sltu_f; end
            4'b1100: begin res_reg = xor_r; flag_reg = xor_f; end
            default: begin
                res_reg = 32'h0000;
                flag_reg = 3'b000;
            end
        endcase
    end
    assign result = res_reg;
    assign flags = flag_reg;

    // always @(*) begin
    //     $display("add: %d, flag: %3b", add_r, add_r);

    //     $display("final: %d, flag: %3b", result, flags);
    // end

endmodule


module add_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire cout, cin, zero;
    reg overflow;
    is_zero_module is_zero_m(result, zero);
    assign {cout, result} = in1 + in2;
    assign cin = result[31] + in1[31] + in2[31];
    always @(*) begin
        if (cin == cout) overflow = 0;
        else overflow = 1;
    end
    assign flags = {zero, result[31], overflow};

endmodule

module sub_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire cout, cin, zero;
    wire[31:0] in2_neg = -in2;
    reg overflow;
    is_zero_module is_zero_m(result, zero);
    assign {cout, result} = in1 + in2_neg;
    assign cin = result[31] + in1[31] + in2_neg[31];
    always @(*) begin
        if (cin == cout) overflow = 0;
        else overflow = 1;
    end
    assign flags = {zero, result[31], overflow};
endmodule

module and_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = in1 & in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module nor_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = in1 ~| in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module or_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = in1 | in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module sll_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero, not_overflow;
    wire[31:0] upper, lower;
    reg[31:0] output_reg;
    reg[3:0] flag_reg;
    is_zero_module is_zero_m1(result, zero);
    is_zero_module is_zero_m2(upper, not_overflow);
    assign {upper, lower} = in1 << in2;
    always @(*) begin
        if (in2 >= 32) begin
            output_reg = 0;
            flag_reg = 3'b001;
        end else begin
            output_reg = lower;
            flag_reg = {zero, result[31], ~not_overflow};
        end
    end
    assign result = output_reg;
    assign flags = flag_reg;
endmodule

module srl_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = in1 >> in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module sra_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = $signed(in1) >>> in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module slt_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    reg[31:0] res_reg;
    reg[2:0] flag_reg;
    always @(*) begin
        if ($signed(in1) < $signed(in2)) begin
            res_reg = 32'h0001;
            flag_reg = 3'b010;
        end else begin
            res_reg = 32'h0000;
            flag_reg = 3'b100;
        end
    end
    assign result = res_reg;
    assign flags = flag_reg;
endmodule

module sltu_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    reg[31:0] res_reg;
    reg[2:0] flag_reg;
    always @(*) begin
        if (in1[31:0] < in2[31:0]) begin
            res_reg = 32'h0001;
            flag_reg = 3'b010;
        end else begin
            res_reg = 32'h0000;
            flag_reg = 3'b100;
        end
    end
    assign result = res_reg;
    assign flags = flag_reg;
endmodule

module xor_module(input[31:0] in1, input[31:0] in2, output[31:0] result, output[2:0] flags);
    wire zero;
    is_zero_module is_zero_m(result, zero);
    assign result = in1 ^ in2;
    assign flags = {zero, result[31], 1'b0};
endmodule

module is_zero_module(input[31:0] result, output zero);
    assign zero = !result;
    // always @(*) begin
    //     $display("result = %32b", result);
    //     $display("zero = %d", zero);
    // end
endmodule