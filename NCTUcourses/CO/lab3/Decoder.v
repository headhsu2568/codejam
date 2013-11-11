`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:30:29 10/12/2009 
// Design Name: 
// Module Name:    Decoder 
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
module Decoder(Clk, Reset, Instruction, Instruction );
	input wire Clk;
	input wire Reset;
	input wire [31:0] Instruction;
	
	wire RegDst;
	wire MemRead;
	wire MemtoReg;
	wire [1:0]ALUOp;
	wire MemWrite;
	wire ALUSrc;
	wire RegWrite;

	Control uut1(.Clk(Clk), .Reset(Reset), .Instruction(Instruction), .RegDst(RegDst), .MemRead(MemRead), .MemtoReg(MemtoReg), .ALUOp(ALUOp), .MemWrite(MemWrite), .ALUSrc(ALUSrc), .RegWrite(RegWrite));
	
	wire [31:0]ReadData1;
	wire [31:0]ReadData2;
	reg [7:0]ReadReg1;
	reg [7:0]ReadReg2;
	reg [7:0]WriteReg;
	reg [31:0]WriteData;
	
	RegisterFile uut2(.Clk(Clk), .Reset(Reset), .Instruction(Instruction), .RegWrite(RegWrite), .ReadReg1(ReadReg1), .ReadReg2(ReadReg2), .WriteReg(WriteReg), .WriteData(WriteData), .ReadData1(ReadData1), .ReadData2(ReadData2));
	
	wire [31:0]ReadData;
	reg [7:0]Addr;
	
	DataMemory uut3(.Clk(Clk), .Reset(Reset), .Instruction(Instruction), .MemWrite(MemWrite), .MemRead(MemRead), .Addr(Addr), .WriteData(WriteData), .ReadData(ReadData));
	
	wire [31:0]ALUResult;
	reg [31:0]A,B;
	
	ALU uut4(.Clk(Clk), .Reset(Reset), .ALUOp(ALUOp), .A(A), .B(B), .ALUResult(ALUResult));
	
	always@(*)
	begin
		ReadReg1<=Instruction[23:16];
		ReadReg2<=Instruction[15:8];
		Addr<=Instruction[23:16];
		A<=ReadData1;
		if(RegDst==1)
			WriteReg<=Instruction[7:0];
		else
			WriteReg<=Instruction[15:8];
		if(MemtoReg==1)
			WriteData<=ReadData;
		else if(MemWrite==1)
			WriteData<=ReadData2;
		else
			WriteData<=ALUResult;
		if(ALUSrc==1)
			B<={8'b0000_0000,Instruction[15:8]};
		else
			B<=ReadData2;
	end
	
endmodule
