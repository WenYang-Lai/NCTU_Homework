// 0316025 賴文揚 0316041 繆穩慶
//Subject:     CO project 2 - ALU Controller
//--------------------------------------------------------------------------------
//Version:     1
//--------------------------------------------------------------------------------
//Writer:      0316025 賴文揚
//----------------------------------------------
//Date:        2016/4/23
//----------------------------------------------
//Description: 
//--------------------------------------------------------------------------------

module ALU_Ctrl(
          funct_i,
          ALUOp_i,
          ALUCtrl_o
          );
          
//I/O ports 
input      [6-1:0] funct_i;
input      [3-1:0] ALUOp_i;

output     [4-1:0] ALUCtrl_o;    
     
	 
//Internal Signals
reg        [4-1:0] ALUCtrl_o;

//Parameter

always@(*)begin
	if (ALUOp_i == 0)begin
		case(funct_i)
			2: ALUCtrl_o <= 4'b1000; //SRL
			6: ALUCtrl_o <= 4'b0101; //SRLV
			32: ALUCtrl_o <= 4'b0000; //ADD
			34: ALUCtrl_o <= 4'b0001; //SUB
			36: ALUCtrl_o <= 4'b0010; //AND
			37: ALUCtrl_o <= 4'b0011; //OR
			42: ALUCtrl_o <= 4'b0100; //SLT
			default: ALUCtrl_o <= 4'b0000; //??
		endcase
	end
	else if (ALUOp_i == 3'b001)begin //ADDi
		ALUCtrl_o <= 4'b0000;
	end
	else if (ALUOp_i == 3'b101)begin //STLi
		ALUCtrl_o <= 4'b0100;  //STL
	end
	else if (ALUOp_i == 3'b100)begin //BEQ
		ALUCtrl_o <= 4'b0001; //SUB
	end
	else if (ALUOp_i == 3'b110)begin
		ALUCtrl_o <= 4'b0110; //LUI
	end
	else if (ALUOp_i == 3'b010)begin //ORI
		ALUCtrl_o <= 4'b0011; 
	end
	else if (ALUOp_i == 3'b011)begin //BNE
		ALUCtrl_o <= 4'b0111;
	end
end
//Select exact operation

endmodule     





                    
                    