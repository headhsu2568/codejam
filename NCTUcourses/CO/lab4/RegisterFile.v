`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:34:32 10/12/2009 
// Design Name: 
// Module Name:    RegisterFile 
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
module RegisterFile(Clk, Reset, Instruction, RegWrite, ReadReg1, ReadReg2, WriteReg, WriteData, ReadData1, ReadData2, R0,R1,R2,R3);
	input wire Clk; 									// clock signal
	input wire Reset;
	input wire [31:0] Instruction;
	input wire RegWrite; 							// write enable
	input wire [1:0] ReadReg1;
	input wire [1:0] ReadReg2;
	input wire [1:0] WriteReg; 					// address bus
	input wire [31:0] WriteData; 					// input data bus
	output reg [31:0] ReadData1;
	output reg [31:0] ReadData2; 					// output data bus
	output reg [31:0] R0,R1,R2,R3;

	always@(posedge Clk)
	begin
		if(Reset==1)
		begin
			R0 <= 0;
			R1 <= 0;
			R2 <= 0;
			R3 <= 0;
		end
		else if(Reset==0 && RegWrite==1)
		begin
			case({WriteReg})
				2'b00:R0[31:0] <= WriteData[31:0];
				2'b01:R1[31:0] <= WriteData[31:0];
				2'b10:R2[31:0] <= WriteData[31:0];
				2'b11:R3[31:0] <= WriteData[31:0];
			endcase
		end
	end
	
	always@(*)
	begin
		if(Reset==0)
		begin
			case({ReadReg1})
				2'b00:ReadData1[31:0] <= R0[31:0];
				2'b01:ReadData1[31:0] <= R1[31:0];
				2'b10:ReadData1[31:0] <= R2[31:0];
				2'b11:ReadData1[31:0] <= R3[31:0];
			endcase
			case({ReadReg2})
				2'b00:ReadData2[31:0] <= R0[31:0];
				2'b01:ReadData2[31:0] <= R1[31:0];
				2'b10:ReadData2[31:0] <= R2[31:0];
				2'b11:ReadData2[31:0] <= R3[31:0];
			endcase
		end
	end

endmodule
