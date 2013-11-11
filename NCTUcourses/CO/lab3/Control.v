`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    18:35:56 11/16/2009 
// Design Name: 
// Module Name:    Control 
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
module Control(Clk, Reset, Instruction, RegDst, MemRead, MemtoReg, ALUOp, MemWrite, ALUSrc, RegWrite);
	
	input wire Clk;
	input wire Reset;
	input wire [31:0]Instruction;
	
	output reg RegDst;
	output reg MemRead;
	output reg MemtoReg;
	output reg [1:0]ALUOp;
	output reg MemWrite;
	output reg ALUSrc;
	output reg RegWrite;

	always@(*)
	begin
		if(Reset==1)
		begin
			MemWrite=0;
			RegWrite=0;
		end
		else if(Instruction[31:24]==8'b1000_0000)				//LOAD
		begin
			RegDst=0;
			MemRead=1;
			MemtoReg=1;
			ALUOp=2'b00;
			MemWrite=0;
			ALUSrc=1;
			RegWrite=1;
		end
		else if(Instruction[31:24]==8'b0100_0000)				//STORE
		begin
			RegDst=0;
			MemRead=0;
			MemtoReg=0;
			ALUOp=2'b00;
			MemWrite=1;
			ALUSrc=1;
			RegWrite=0;
		end
		else if(Instruction[31:24]==8'b0010_0000)				//ADD
		begin
			RegDst=1;
			MemRead=1;
			MemtoReg=0;
			ALUOp=2'b00;
			MemWrite=0;
			ALUSrc=0;
			RegWrite=1;
		end
		else if(Instruction[31:24]==8'b0001_0000)				//SUB
		begin
			RegDst=1;
			MemRead=1;
			MemtoReg=0;
			ALUOp=2'b01;
			MemWrite=0;
			ALUSrc=0;
			RegWrite=1;
		end
		else if(Instruction[31:24]==8'b0000_1000)				//ShiftLeft
		begin
			RegDst=1;
			MemRead=1;
			MemtoReg=0;
			ALUOp=2'b10;
			MemWrite=0;
			ALUSrc=1;
			RegWrite=1;
		end
		else if(Instruction[31:24]==8'b0000_0100)				//ShiftRight
		begin
			RegDst=1;
			MemRead=1;
			MemtoReg=0;
			ALUOp=2'b11;
			MemWrite=0;
			ALUSrc=1;
			RegWrite=1;
		end
	end

endmodule
