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
module Decoder(Clk, Reset, Instruction);
	input wire Clk;
	input wire Reset;
	input wire [31:0] Instruction;
	 
	reg MemWrite; 							// write enable
	reg MemRead; 							// read enable
	reg [3:0] Addr; 						// address bus
	reg [31:0] WriteMemData;			// input data bus
	wire [31:0] ReadData;
	 
	DataMemory uut2(
		.Clk(Clk),
		.Reset(Reset),
		.Instruction(Instruction),
		.MemWrite(MemWrite),
		.MemRead(MemRead),
		.Addr(Addr),
		.WriteData(WriteMemData),
		.ReadData(ReadData)
	);
	
	reg RegWrite;
	reg [1:0] WriteReg;
	reg [1:0] ReadReg1;
	reg [1:0] ReadReg2;
	reg [31:0] WriteRegData;
	wire [31:0] ReadData1;
	wire [31:0] ReadData2;
	
	RegisterFile uut3(
		.Clk(Clk),
		.Reset(Reset),
		.Instruction(Instruction),
		.RegWrite(RegWrite),
		.WriteReg(WriteReg),
		.ReadReg1(ReadReg1),
		.ReadReg2(ReadReg2),
		.WriteData(WriteRegData),
		.ReadData1(ReadData1),
		.ReadData2(ReadData2)
	);

	always@(posedge Clk)
	begin
		if(Reset==1)
		begin
			MemWrite <= 0;
			MemRead <= 0;
			RegWrite <= 0;
		end
		else if(Instruction[31:24]==8'b1000_0000)
		begin
			MemWrite <= 0;
			MemRead <= 1;
			RegWrite <= 1;
		end
		else if(Instruction[31:24]==8'b0100_0000)
		begin
			MemWrite <= 1;
			MemRead <= 0;
			RegWrite <= 0;
		end
	end
	
	always@(posedge Clk)
	begin
		if(Reset==0)
		begin
			Addr[3:0] <= Instruction[19:16];
		end
		else if(Reset==0 && Instruction[31:24]==8'b1000_0000)
		begin
			WriteReg <= Instruction[9:8];
			WriteRegData <= ReadData;
		end
		else if(Reset==0 && Instruction[31:24]==8'b0100_0000)
		begin
			ReadReg1 <= Instruction[9:8];
			ReadReg2 <= Instruction[1:0];
			WriteMemData <= ReadData1;
		end
	end

endmodule
