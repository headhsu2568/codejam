`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    22:35:35 11/01/2009 
// Design Name: 
// Module Name:    ALU 
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
module ALU(Clk, Reset, ALUOp, A, B, ALUResult);
	input wire Clk; 									// clock signal
	input wire Reset;
	input wire [1:0]ALUOp;							// control signal
	input wire [31:0]A,B;							// operands
	output reg [31:0]ALUResult;					// result
	
	always@(*)
	begin
		if(Reset==1)
		begin
			ALUResult <= 0;
		end
		else if(Clk==0)
		begin
			if(ALUOp == 2'b00)
			begin
				ALUResult <= (A+B);
			end
			else if(ALUOp == 2'b01)
			begin
				ALUResult <= (A-B);
			end
			else if(ALUOp == 2'b10)
			begin
				ALUResult <= (A << B);
			end
			else if(ALUOp == 2'b11)
			begin
				ALUResult <= (A >> B);
			end
		end
	end

endmodule
