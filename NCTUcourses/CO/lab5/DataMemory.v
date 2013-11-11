`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:36:53 10/12/2009 
// Design Name: 
// Module Name:    DataMemory 
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
module DataMemory(Clk, Reset, MemWrite, MemRead, Addr, WriteMemData, ReadData);
	input wire Clk; 								// clock signal
	input wire Reset;
	input wire MemWrite; 						// write enable
	input wire MemRead; 							// read enable
	input wire [7:0] Addr; 						// address bus
	input wire [31:0] WriteMemData;			// input data bus
	output reg [31:0] ReadData; 				// output data bus
	reg [31:0] M0,M1,M2,M3,M4,M5,M6,M7;
	
	always@(posedge Clk)
	begin
		if(Reset==1)
		begin
			M0 <= 0;
			M1 <= 1;
			M2 <= 2;
			M3 <= 3;
			M4 <= 4;
			M5 <= 5;
			M6 <= 6;
			M7 <= 7;
		end
		else if(Reset==0 && MemWrite==1 && MemRead==0)
		begin
			case({Addr[7:0]})
				8'b0000_0000:M0[31:0] <= WriteMemData[31:0];
				8'b0000_0001:M1[31:0] <= WriteMemData[31:0];
				8'b0000_0010:M2[31:0] <= WriteMemData[31:0];
				8'b0000_0011:M3[31:0] <= WriteMemData[31:0];
				8'b0000_0100:M4[31:0] <= WriteMemData[31:0];
				8'b0000_0101:M5[31:0] <= WriteMemData[31:0];
				8'b0000_0110:M6[31:0] <= WriteMemData[31:0];
				8'b0000_0111:M7[31:0] <= WriteMemData[31:0];
			endcase
		end
	end
	
	always@(*)
	begin
	if(Reset==0 && MemRead==1 && MemWrite==0 && Clk==0)
		begin
			case({Addr[7:0]})
				8'b0000_0000:ReadData[31:0] <= M0[31:0];
				8'b0000_0001:ReadData[31:0] <= M1[31:0];
				8'b0000_0010:ReadData[31:0] <= M2[31:0];
				8'b0000_0011:ReadData[31:0] <= M3[31:0];
				8'b0000_0100:ReadData[31:0] <= M4[31:0];
				8'b0000_0101:ReadData[31:0] <= M5[31:0];
				8'b0000_0110:ReadData[31:0] <= M6[31:0];
				8'b0000_0111:ReadData[31:0] <= M7[31:0];
			endcase
		end
	end

endmodule
