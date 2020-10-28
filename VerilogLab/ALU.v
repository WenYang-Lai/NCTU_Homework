// 0316025 賴文揚 0316041 繆穩慶
//Subject:     CO project 2 - ALU
//--------------------------------------------------------------------------------
//Version:     1
//--------------------------------------------------------------------------------
//Writer:      0316025 賴文揚
//----------------------------------------------
//Date:        2016/4/23
//----------------------------------------------
//Description: 
//--------------------------------------------------------------------------------

module ALU(
    src1_i,
	src2_i,
	ctrl_i,
	result_o,
	zero_o
	);
     
//I/O ports
input  signed[32-1:0]  src1_i;
input  signed[32-1:0]  src2_i;
input  signed[4-1:0]   ctrl_i;

output [32-1:0]	 result_o;
output           zero_o;

//Internal signals
reg    [32-1:0]  result_o;
wire             zero_o;

//Parameter

//Main function

always@(*)begin
	case(ctrl_i)
		4'b0000: result_o <= src1_i + src2_i; //ADD
		4'b0001: result_o <= src1_i - src2_i; //SUB
		4'b0010: result_o <= src1_i & src2_i; //AND
		4'b0011: result_o <= src1_i | src2_i; //OR
		4'b0100: result_o <= src1_i < src2_i ? 1 : 0; //SLT
		4'b1000: result_o <= src2_i >> src1_i; //SRL
		4'b0101: result_o <= src2_i >> src1_i; //SRLV
		4'b0111: result_o <= src1_i==src2_i ? 1 : 0;
		4'b0110: result_o <= src2_i<<16;
		default: result_o <= result_o;
	endcase
end
assign zero_o = result_o == 0 ? 1 : 0;
endmodule





                    
                    