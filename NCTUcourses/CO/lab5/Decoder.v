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
module Decoder(Clk, Reset, InstructionMemory0, InstructionMemory1, InstructionMemory2, InstructionMemory3, InstructionMemory4, InstructionMemory5, InstructionMemory6, InstructionMemory7);
	input wire Clk;
	input wire Reset;
	input wire [31:0] InstructionMemory0,InstructionMemory1,InstructionMemory2,InstructionMemory3,InstructionMemory4,InstructionMemory5,InstructionMemory6,InstructionMemory7;
	wire [31:0] Instruction;
	
//	Control Signal
	wire RegDst;
	wire MemRead;
	wire MemtoReg;
	wire [1:0]ALUOp;
	wire MemWrite;
	wire ALUSrc;
	wire RegWrite;
	
// IF/ID
	reg [31:0] ifid_instr ;
	
// ID/EX
	reg idex_wb_RegWrite;
	reg idex_m_MemRead;
	reg idex_m_MemWrite;
	reg idex_m_MemtoReg;
	reg idex_ex_RegDst;
	reg idex_ex_ALUSrc;
	reg [1:0]idex_ex_ALUOp;
	reg [31:0] idex_ReadData1;
	reg [31:0] idex_ReadData2;
	reg [31:0] idex_signex;
	reg [7:0] idex_ReadReg1;
	reg [7:0] idex_ReadReg2;
	reg [7:0] idex_rs;
	reg [7:0] idex_rd;
	reg [7:0] idex_rt;
	
// EX/MEM
	reg exmem_wb_RegWrite;
	reg exmem_m_MemRead;
	reg exmem_m_MemWrite;
	reg exmem_m_MemtoReg;
	reg [7:0] exmem_WriteReg;
	reg [7:0] exmem_ReadReg1;
	reg [7:0] exmem_ReadReg2;
	reg [31:0] exmem_ALUResult;
	reg [7:0] exmem_Addr;
	reg [31:0] exmem_WriteData;
// MEM/WB
	reg memwb_RegWrite;
	reg [7:0]memwb_WriteReg;
	reg [31:0]memwb_WriteData;
	wire PCWrite;
	
	PC uut0(.Clk(Clk), .Reset(Reset), .PCWrite(PCWrite), .InstructionMemory0(InstructionMemory0), .InstructionMemory1(InstructionMemory1), .InstructionMemory2(InstructionMemory2), .InstructionMemory3(InstructionMemory3), .InstructionMemory4(InstructionMemory4), .InstructionMemory5(InstructionMemory5), .InstructionMemory6(InstructionMemory6), .InstructionMemory7(InstructionMemory7), .Instruction(Instruction));

	Control uut1(.Clk(Clk), .Reset(Reset), .Instruction(ifid_instr), .RegDst(RegDst), .MemRead(MemRead), .MemtoReg(MemtoReg), .ALUOp(ALUOp), .MemWrite(MemWrite), .ALUSrc(ALUSrc), .RegWrite(RegWrite));
	
	wire [31:0]ReadData1;
	wire [31:0]ReadData2;
	reg [7:0]ReadReg1;
	reg [7:0]ReadReg2;
	reg [7:0]WriteReg;
	reg [31:0]WriteRegData;
	
	RegisterFile uut2(.Clk(Clk), .Reset(Reset), .RegWrite(memwb_RegWrite), .ReadReg1(ReadReg1), .ReadReg2(ReadReg2), .WriteReg(WriteReg), .WriteRegData(WriteRegData), .ReadData1(ReadData1), .ReadData2(ReadData2));
	
	reg [31:0]WriteMemData;
	wire [31:0]ReadData;
	reg [7:0]Addr;
	
	DataMemory uut3(.Clk(Clk), .Reset(Reset), .MemWrite(exmem_m_MemWrite), .MemRead(exmem_m_MemRead), .Addr(Addr), .WriteMemData(WriteMemData), .ReadData(ReadData));
	
	wire [31:0]ALUResult;
	reg [31:0]A,B;
	
	ALU uut4(.Clk(Clk), .Reset(Reset), .ALUOp(idex_ex_ALUOp), .A(A), .B(B), .ALUResult(ALUResult));
	
	wire ifid_Write;
	wire idex_Flush;
	HazardDetect uut5(.Clk(Clk), .Reset(Reset), .Instruction(ifid_instr), .idex_rt(idex_rt), .idex_m_MemRead(idex_m_MemRead), .ifid_Write(ifid_Write), .PCWrite(PCWrite), .idex_Flush(idex_Flush));
	
	wire [1:0]ForwardA;
	wire [1:0]ForwardB;
	wire ForwardSW;
	Forwarding uut6(.Clk(Clk), .Reset(Reset), .idex_rs(idex_rs), .idex_rt(idex_rt), .exmem_WriteReg(exmem_WriteReg), .memwb_WriteReg(memwb_WriteReg), .exmem_ReadReg2(exmem_ReadReg2), .exmem_wb_RegWrite(exmem_wb_RegWrite), .idex_m_MemRead(idex_m_MemRead), .idex_m_MemWrite(idex_m_MemWrite), .exmem_m_MemWrite(exmem_m_MemWrite), .memwb_RegWrite(memwb_RegWrite), .ForwardA(ForwardA), .ForwardB(ForwardB), .ForwardSW(ForwardSW));
	
	always@(posedge Clk)
	begin
	if(ifid_Write==1)
		ifid_instr<=Instruction;
	
	if(idex_Flush==1)
	begin
		idex_wb_RegWrite<=0;
		idex_m_MemRead<=0;
		idex_m_MemWrite<=0;
		idex_m_MemtoReg<=0;
		idex_ex_RegDst<=0;
		idex_ex_ALUSrc<=0;
		idex_ex_ALUOp<=2'b00;
		idex_ReadData1<=32'b0000_0000_0000_0000_0000_0000_0000_0000;
		idex_ReadData2<=32'b0000_0000_0000_0000_0000_0000_0000_0000;
		idex_signex<=32'b0000_0000_0000_0000_0000_0000_0000_0000;
		idex_ReadReg1<=8'b1111_1111;
		idex_ReadReg2<=8'b1111_1111;
		idex_rs<=8'b1111_1111;
		idex_rd<=8'b1111_1111;
		idex_rt<=8'b1111_1111;
	end
	else
	begin
		idex_wb_RegWrite<=RegWrite;
		idex_m_MemRead<=MemRead;
		idex_m_MemWrite<=MemWrite;
		idex_m_MemtoReg<=MemtoReg;
		idex_ex_RegDst<=RegDst;
		idex_ex_ALUSrc<=ALUSrc;
		idex_ex_ALUOp<=ALUOp;
		idex_ReadData1<=ReadData1;
		idex_ReadData2<=ReadData2;
		idex_signex<={24'b0000_0000_0000_0000_0000_0000,ifid_instr[15:8]};
		idex_ReadReg1<=ifid_instr[23:16];
		idex_ReadReg2<=ifid_instr[15:8];
		idex_rs<=ifid_instr[23:16];
		idex_rd<=ifid_instr[7:0];
		idex_rt<=ifid_instr[15:8];
	end
		
		exmem_wb_RegWrite<=idex_wb_RegWrite;
		exmem_m_MemRead<=idex_m_MemRead;
		exmem_m_MemWrite<=idex_m_MemWrite;
		exmem_m_MemtoReg<=idex_m_MemtoReg;
		if(idex_ex_RegDst==1)
			exmem_WriteReg<=idex_rd;
		else
			exmem_WriteReg<=idex_rt;
		exmem_ReadReg1<=idex_ReadReg1;
		exmem_ReadReg2<=idex_ReadReg2;
		exmem_ALUResult<=ALUResult;
		exmem_Addr<=idex_ReadReg1;
		exmem_WriteData<=idex_ReadData2;
		
		memwb_RegWrite<=exmem_wb_RegWrite;
		memwb_WriteReg<=exmem_WriteReg;
		if(exmem_m_MemtoReg==1)
			memwb_WriteData<=ReadData;
		else
			memwb_WriteData<=exmem_ALUResult;
	end
	
	always@(*)
	begin
		ReadReg1<=ifid_instr[23:16];
		ReadReg2<=ifid_instr[15:8];
		Addr<=exmem_Addr;
		if(ForwardA==2'b00)
			A<=idex_ReadData1;
		else if(ForwardA==2'b10)
			A<=exmem_ALUResult;
		else if(ForwardA==2'b01)
			A<=memwb_WriteData;
		if(ForwardB==2'b00)
		begin
			if(idex_ex_ALUSrc==1)
				B<=idex_signex;
			else
				B<=idex_ReadData2;
		end
		else if(ForwardB==2'b10)
			B<=exmem_ALUResult;
		else if(ForwardB==2'b01)
			B<=memwb_WriteData;
		WriteReg<=memwb_WriteReg;
		WriteRegData<=memwb_WriteData;
		if(ForwardSW==1)
			WriteMemData<=memwb_WriteData;
		else
			WriteMemData<=exmem_WriteData;
	end
	
endmodule
