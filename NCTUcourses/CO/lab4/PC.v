`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   04:46:35 10/12/2009
// Design Name:   PC
// Module Name:   
// Project Name: 
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: Decoder
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module PC();

	// Inputs
	reg Clk;
	reg Reset;
	reg [31:0] Instruction;
	reg [31:0] InstructionMemory[0:7];
	reg [2:0] PC;

	// Instantiate the Unit Under Test (UUT)
	Decoder uut (
		.Clk(Clk), 
		.Reset(Reset), 
		.Instruction(Instruction)
	);
	
	always@(posedge Clk)
	begin
		if(!Reset)
		begin
			case(PC)
				0:Instruction<=InstructionMemory[0];
				1:Instruction<=InstructionMemory[1];
				2:Instruction<=InstructionMemory[2];
				3:Instruction<=InstructionMemory[3];
				4:Instruction<=InstructionMemory[4];
				5:Instruction<=InstructionMemory[5];
				6:Instruction<=InstructionMemory[6];
				7:Instruction<=InstructionMemory[7];
			endcase
			PC <= PC+1;
		end
	end
	
	initial begin
		// Initialize Inputs
		Clk = 0;
		Reset = 0;
		Instruction = 0;
		InstructionMemory[0] = 32'b10000000_00000001_00000000_00000000;
		InstructionMemory[1] = 32'b10000000_00000110_00000001_00000000;
		InstructionMemory[2] = 32'b00100000_00000000_00000001_00000010;
		InstructionMemory[3] = 32'b00001000_00000010_00000101_00000011;
		InstructionMemory[4] = 32'b00010000_00000011_00000001_00000001;
		InstructionMemory[5] = 32'b00000100_00000000_00000010_00000010;
		InstructionMemory[6] = 32'b01000000_00000101_00000001_00000000;
		InstructionMemory[7] = 32'b01000000_00000011_00000010_00000000;
		PC = 0;

		#10 Reset=1;
		#20 Reset=0;

		// Add stimulus here
	end
	
	always
		#10 Clk = ~Clk;
      
endmodule

