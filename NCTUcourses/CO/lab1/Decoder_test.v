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

module Decoder_test;

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
		#60 Instruction=32'b10000000_00000000_00000011_00000000;
		#60 Instruction=32'b10000000_00000001_00000010_00000000;
		#60 Instruction=32'b10000000_00000010_00000001_00000000;
		#60 Instruction=32'b10000000_00000011_00000000_00000000;
		
		#60 Instruction=32'b01000000_00000111_00000001_00000000;
		#60 Instruction=32'b01000000_00000110_00000000_00000000;
		#60 Instruction=32'b01000000_00000101_00000010_00000000;
		#60 Instruction=32'b01000000_00000100_00000011_00000000;    
      
		// Add stimulus here
	end
	
	always
		#10 Clk = ~Clk;
      
endmodule

