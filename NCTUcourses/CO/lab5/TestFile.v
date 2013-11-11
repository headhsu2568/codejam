`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    03:46:34 12/28/2009 
// Design Name: 
// Module Name:    TestFile 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module TestFile();
	// Inputs
	reg Clk;
	reg Reset;
	reg [31:0] InstructionMemory[0:7];

	// Instantiate the Unit Under Test (UUT)
	Decoder uut (
		.Clk(Clk), 
		.Reset(Reset), 
		.InstructionMemory0(InstructionMemory[0]),
		.InstructionMemory1(InstructionMemory[1]),
		.InstructionMemory2(InstructionMemory[2]),
		.InstructionMemory3(InstructionMemory[3]),
		.InstructionMemory4(InstructionMemory[4]),
		.InstructionMemory5(InstructionMemory[5]),
		.InstructionMemory6(InstructionMemory[6]),
		.InstructionMemory7(InstructionMemory[7])
	);
	
	initial begin
		// Initialize Inputs
		Clk = 0;
		Reset = 0;
		InstructionMemory[0] = 32'b10000000_00000001_00000000_00000000;
		InstructionMemory[1] = 32'b10000000_00000110_00000001_00000000;
		InstructionMemory[2] = 32'b00100000_00000000_00000001_00000010;
		InstructionMemory[3] = 32'b00001000_00000010_00000101_00000011;
		InstructionMemory[4] = 32'b00010000_00000011_00000001_00000001;
		InstructionMemory[5] = 32'b00000100_00000000_00000010_00000010;
		InstructionMemory[6] = 32'b01000000_00000011_00000010_00000000;
		InstructionMemory[7] = 32'b01000000_00000101_00000001_00000000;

		#10 Reset=1;
		#20 Reset=0;

		// Add stimulus here
	end
	
	always
		#10 Clk = ~Clk;

endmodule
