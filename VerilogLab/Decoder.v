// 0316025 賴文揚 0316041 繆穩慶
//Subject:     CO project 2 - Decoder
//--------------------------------------------------------------------------------
//Version:     1
//--------------------------------------------------------------------------------
//Writer:      0316025 賴文揚
//----------------------------------------------
//Date:        2016/4/23
//----------------------------------------------
//Description: 
//--------------------------------------------------------------------------------

module Decoder(
    instr_op_i,
	RegWrite_o,
	ALU_op_o,
	ALUSrc_o,
	RegDst_o,
	Branch_o
	);
     
//I/O ports
input  [6-1:0] instr_op_i;

output         RegWrite_o;
output [3-1:0] ALU_op_o;
output         ALUSrc_o;
output         RegDst_o;
output         Branch_o;
 
//Internal Signals
reg    [3-1:0] ALU_op_o;
reg            ALUSrc_o;
reg            RegWrite_o;
reg            RegDst_o;
reg            Branch_o;

//Parameter

always@(*)begin
	case(instr_op_i)
		0:begin 
			RegDst_o <=1; ALUSrc_o <=0;  RegWrite_o <= 1; ALU_op_o <= 3'b000; Branch_o <=0; 
		end 
		4:begin //BEQ
			RegDst_o <=0; ALUSrc_o <=0;  RegWrite_o <= 0; ALU_op_o <= 3'b100; Branch_o <=1;
		end
		5:begin //BNE
			RegDst_o <=0; ALUSrc_o <=0; RegWrite_o <= 0; ALU_op_o <= 3'b011;  Branch_o <=1;
		end
		8:begin //ADDi
			RegDst_o <=0; ALUSrc_o <=1;  RegWrite_o <= 1; ALU_op_o <= 3'b001; Branch_o <=0;
		end
		10:begin //SLTI
			RegDst_o <=0; ALUSrc_o <=1;  RegWrite_o <= 1; ALU_op_o <= 3'b101; Branch_o <=0;
		end
		13:begin //ORI
			RegDst_o <=0; ALUSrc_o <=1; RegWrite_o <= 1; ALU_op_o <= 3'b010;  Branch_o <=0;
		end
		15:begin //LUI
			RegDst_o <=0; ALUSrc_o <=1;  RegWrite_o <= 1; ALU_op_o <= 3'b110; Branch_o <=0;
		end
		default: begin 
			RegDst_o <=RegDst_o; ALUSrc_o <=ALUSrc_o;  RegWrite_o <= 0; ALU_op_o <= ALU_op_o; Branch_o <= 0;
		end
	endcase
end
//Main function

endmodule





                    
                    