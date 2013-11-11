`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   04:46:35 10/12/2009
// Design Name:   PC
// Module Name:   
// Project Name: 
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: Decoder
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module PC(Clk, Reset, PCWrite, InstructionMemory0, InstructionMemory1, InstructionMemory2, InstructionMemory3, InstructionMemory4, InstructionMemory5, InstructionMemory6, InstructionMemory7, Instruction);

	input wire Clk;
	input wire Reset;
	input wire PCWrite;
	output reg [31:0] Instruction;
	input wire [31:0] InstructionMemory0,InstructionMemory1,InstructionMemory2,InstructionMemory3,InstructionMemory4,InstructionMemory5,InstructionMemory6,InstructionMemory7;
	reg [2:0] PC;
	
	always@(negedge Clk)
	begin
		if(Reset==1)
		begin
			PC <=0;
		end
		else if(PCWrite==1)
		begin
			case(PC)
				0:Instruction<=InstructionMemory0;
				1:Instruction<=InstructionMemory1;
				2:Instruction<=InstructionMemory2;
				3:Instruction<=InstructionMemory3;
				4:Instruction<=InstructionMemory4;
				5:Instruction<=InstructionMemory5;
				6:Instruction<=InstructionMemory6;
				7:Instruction<=InstructionMemory7;
			endcase
			PC <= PC+1;
		end
	end
      
endmodule

