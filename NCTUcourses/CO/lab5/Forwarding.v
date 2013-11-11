`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    04:40:27 12/28/2009 
// Design Name: 
// Module Name:    Forwarding 
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
module Forwarding(Clk, Reset, idex_rs, idex_rt, exmem_WriteReg, memwb_WriteReg, exmem_ReadReg2, exmem_wb_RegWrite, idex_m_MemRead, idex_m_MemWrite, exmem_m_MemWrite, memwb_RegWrite, ForwardA, ForwardB, ForwardSW);
	input wire Clk,Reset;
	input wire [7:0] idex_rs, idex_rt, exmem_WriteReg, memwb_WriteReg, exmem_ReadReg2;
	input wire exmem_wb_RegWrite, exmem_m_MemWrite, memwb_RegWrite, idex_m_MemRead, idex_m_MemWrite;
	output reg [1:0]ForwardA;
	output reg [1:0]ForwardB;
	output reg ForwardSW;
	
	always@(*)
	begin
		if(Reset==1)
		begin
			ForwardA<=2'b00;
			ForwardB<=2'b00;
		end
		if(Clk==1 && Reset!=1 && idex_m_MemWrite!=1 && idex_m_MemRead!=1)
		begin
			if(exmem_wb_RegWrite==1 && exmem_WriteReg==idex_rs)
			begin
				ForwardA <=2'b10;
			end
			else if(memwb_RegWrite==1 && memwb_WriteReg==idex_rs)
			begin
				ForwardA <=2'b01;
			end
			else
			begin
				ForwardA <=2'b00;
			end
			
			if(exmem_wb_RegWrite==1 && exmem_WriteReg==idex_rt)
			begin
				ForwardB <=2'b10;
			end
			else if(memwb_RegWrite==1 && memwb_WriteReg==idex_rt)
			begin
				ForwardB <=2'b01;
			end
			else
			begin
				ForwardB <=2'b00;
			end
		end
		if(Clk==1 && Reset!=1 && exmem_m_MemWrite==1)
		begin
			if(exmem_ReadReg2==memwb_WriteReg && memwb_RegWrite==1)
				ForwardSW<=1;
			else
				ForwardSW<=0;
		end
	end

endmodule
