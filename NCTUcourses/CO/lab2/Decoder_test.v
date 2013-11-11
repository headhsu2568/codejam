`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   04:46:35 10/12/2009
// Design Name:   Decoder
// Module Name:   E:/Work/co_lab/lab1/Decoder_test.v
// Project Name:  lab1
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

module Decoder_test();

	// Inputs
	reg Clk;
	reg Reset;
	reg [31:0] Instruction;

	// Instantiate the Unit Under Test (UUT)
	Decoder uut (
		.Clk(Clk), 
		.Reset(Reset), 
		.Instruction(Instruction)
	);
	
	initial begin
		// Initialize Inputs
		Clk = 0;
		Reset = 0;
		Instruction = 0;	

		#10 Reset=1;
		#10 Reset=0;
		#100 Instruction=32'b00100000_00000000_00000001_00000011; // (ADD R0 R1 R3)
		#100 Instruction=32'b00100000_00000010_00000000_00000001; // (ADD R2 R0 R1)
		#100 Instruction=32'b00010000_00000001_00000011_00000010; // (SUB R1 R3 R2)
		#100 Instruction=32'b00010000_00000011_00000010_00000000; // (SUB R3 R2 R0)
		#100 Instruction=32'b00001000_00000000_00000010_00000010; // (SL R0 2 R2)
		#100 Instruction=32'b00000100_00000001_00000011_00000000; // (SR R1 3 R0)  
      
		// Add stimulus here
	end
	
	always
		#10 Clk = ~Clk;
      
endmodule

