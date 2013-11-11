`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    01:48:42 12/28/2009 
// Design Name: 
// Module Name:    HazardDetect 
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
module HazardDetect(Clk, Reset, Instruction, idex_rt, idex_m_MemRead, ifid_Write, PCWrite, idex_Flush);
	input wire Clk;
	input wire Reset;
	input wire [31:0]Instruction;
	input wire [7:0]idex_rt;
	input wire idex_m_MemRead;
	output reg ifid_Write,PCWrite,idex_Flush;
	
	always@(*)
	begin
		if(Reset==1)
		begin
			ifid_Write<=1;
			PCWrite<=1;
			idex_Flush<=0;
		end
		else if(idex_m_MemRead==1 && Instruction[31:24]!=8'b1000_0000 && Instruction[31:24]!=8'b0100_0000)
		begin
			if(idex_rt==Instruction[23:16] || idex_rt==Instruction[15:8])
			begin
				ifid_Write<=0;
				PCWrite<=0;
				idex_Flush<=1;
			end
			else
			begin
				ifid_Write<=1;
				PCWrite<=1;
				idex_Flush<=0;
			end
		end
		else
		begin
			ifid_Write<=1;
			PCWrite<=1;
			idex_Flush<=0;
		end
	end

endmodule
