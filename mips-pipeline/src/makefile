test:compile; vvp CPU;
compile: cpu.v test_cpu.v; 
		 iverilog -o CPU cpu.v test_cpu.v control.v alu_module.v utils.v hazard.v;