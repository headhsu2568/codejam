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
module RegisterFile(Clk, Reset, RegWrite, ReadReg1, ReadReg2, WriteReg, WriteRegData, ReadData1, ReadData2, R0,R1,R2,R3);
	input wire Clk; 									// clock signal
	input wire Reset;
	input wire RegWrite; 							// write enable
	input wire [7:0] ReadReg1;
	input wire [7:0] ReadReg2;
	input wire [7:0] WriteReg; 					// address bus
	input wire [31:0] WriteRegData; 				// input data bus
	output reg [31:0] ReadData1;
	output reg [31:0] ReadData2; 					// output data bus
	output reg [31:0] R0,R1,R2,R3;

	always@(*)
	begin
		if(Reset==1)
		begin
			R0 <= 0;
			R1 <= 0;
			R2 <= 0;
			R3 <= 0;
		end
		else if(Reset==0 && RegWrite==1 && Clk==1)
		begin
			case({WriteReg})
				8'b0000_0000:R0[31:0] <= WriteRegData[31:0];
				8'b0000_0001:R1[31:0] <= WriteRegData[31:0];
				8'b0000_0010:R2[31:0] <= WriteRegData[31:0];
				8'b0000_0011:R3[31:0] <= WriteRegData[31:0];
			endcase
		end
	end
	
	always@(*)
	begin
		if(Reset==0 && Clk==0)
		begin
			case({ReadReg1})
				8'b0000_0000:ReadData1[31:0] <= R0[31:0];
				8'b0000_0001:ReadData1[31:0] <= R1[31:0];
				8'b0000_0010:ReadData1[31:0] <= R2[31:0];
				8'b0000_0011:ReadData1[31:0] <= R3[31:0];
			endcase
			case({ReadReg2})
				8'b0000_0000:ReadData2[31:0] <= R0[31:0];
				8'b0000_0001:ReadData2[31:0] <= R1[31:0];
				8'b0000_0010:ReadData2[31:0] <= R2[31:0];
				8'b0000_0011:ReadData2[31:0] <= R3[31:0];
			endcase
		end
	end

endmodule
