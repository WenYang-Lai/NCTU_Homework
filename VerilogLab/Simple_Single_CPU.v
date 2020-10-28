// 0316025 賴文揚 0316041 繆穩慶
//Subject:     CO project 2 - Simple Single CPU
//--------------------------------------------------------------------------------
//Version:     1
//--------------------------------------------------------------------------------
//Writer:      0316025 賴文揚
//----------------------------------------------
//Date:        2016/4/23
//----------------------------------------------
//Description: 
//--------------------------------------------------------------------------------
module Simple_Single_CPU(
        clk_i,
		rst_i
		);
		
//I/O port
input         clk_i;
input         rst_i;

//Internal Signles
wire [32-1:0] pc_out_i,pc_out_o,pc_in_i, 
              interger_four, 
			  Adder1_i,Adder1_o, 
			  Adder2_i1,Adder2_i2,Adder2_o,
			  pc_addr_i,instr_o,
			  data0_PCSi,data1_PCSi,data_PCSo,
			  data0_ALUSi,data1_ALUSi,data_ALUSo,
			  data_SEo,
			  data_SL2i,data_SL2o,
			  RDdata_i, RSdata_o,  RTdata_o, 
			  src1_ALUi,src2_ALUi,result_ALUo,
			  data0_ALUSi2,data1_ALUSi2,data_ALUS2o; 
			  
wire  RegWrite_o, RegDst_o, Branch_o, select_ALUSi, select_PCSi, select_WRi, RegWrite_i, zero_ALUo, select_ALUSi2, ALUSrc_o;

wire [6-1:0] instr_op_i ,funct_i ;

wire [4-1:0] ALUCtrl_o, ctrl_ALUi;

wire [3-1:0] ALUOp_i, ALU_op_o;

wire [5-1:0] data0_WRi ,data1_WRi ,RSaddr_i ,RTaddr_i  ,RDaddr_i ,data_WRo ;

wire [16-1:0] data_SEi;

assign data0_ALUSi2 = RSdata_o;
assign data1_ALUSi2 = instr_o[10:6];
assign select_ALUSi2 = ALUCtrl_o == 4'b1000 ? 1 : 0;

assign select_PCSi = zero_ALUo & Branch_o;

assign RDdata_i = result_ALUo;

assign select_ALUSi = ALUSrc_o;

assign src1_ALUi = data_ALUS2o;
assign src2_ALUi = data_ALUSo;

assign ctrl_ALUi = ALUCtrl_o;

assign data0_PCSi = Adder1_o;
assign data1_PCSi = Adder2_o;

assign pc_addr_i = pc_out_o;

assign Adder1_i = pc_out_o;

assign Adder2_i1 = Adder1_o;
assign Adder2_i2 = data_SL2o;

assign data0_ALUSi = RTdata_o;
assign data1_ALUSi = data_SEo;

assign ALUOp_i = ALU_op_o;

assign pc_in_i = data_PCSo; 

assign data_SL2i = data_SEo;

assign select_WRi = RegDst_o;
assign RegWrite_i = RegWrite_o;

assign data_SEi = instr_o [15:0];

assign RDaddr_i = data_WRo;

assign funct_i = instr_o[5:0];
assign RTaddr_i = instr_o[20:16];
assign RSaddr_i = instr_o[25:21];
assign data0_WRi = instr_o[20:16];
assign data1_WRi = instr_o[15:11];
assign instr_op_i = instr_o[31:26];

assign interger_four = 32'd4;
//Greate componentes
ProgramCounter PC(
        .clk_i(clk_i),      
	    .rst_i (rst_i),     
	    .pc_in_i(pc_in_i) ,   
	    .pc_out_o(pc_out_o) 
	    );
	
Adder Adder1(
        .src1_i(interger_four),     
	    .src2_i(Adder1_i),     
	    .sum_o(Adder1_o)    
	    );
	
Instr_Memory IM(
        .pc_addr_i(pc_addr_i),  
	    .instr_o(instr_o)
	    );

MUX_2to1 #(.size(5)) Mux_Write_Reg(
        .data0_i(data0_WRi),
        .data1_i(data1_WRi),
        .select_i(select_WRi),
        .data_o(data_WRo)
        );	
		
Reg_File RF(
        .clk_i(clk_i),      
	    .rst_i(rst_i) ,     
        .RSaddr_i(RSaddr_i) ,  
        .RTaddr_i(RTaddr_i) ,  
        .RDaddr_i(RDaddr_i) ,  
        .RDdata_i(RDdata_i) , 
        .RegWrite_i (RegWrite_i),
        .RSdata_o(RSdata_o) ,  
        .RTdata_o(RTdata_o)   
        );
	
Decoder Decoder(
        .instr_op_i(instr_op_i), 
	    .RegWrite_o(RegWrite_o), 
	    .ALU_op_o(ALU_op_o),   
	    .ALUSrc_o(ALUSrc_o),   
	    .RegDst_o(RegDst_o),   
		.Branch_o(Branch_o)   
	    );

ALU_Ctrl AC(
        .funct_i(funct_i),   
        .ALUOp_i(ALUOp_i),   
        .ALUCtrl_o(ALUCtrl_o) 
        );
	
Sign_Extend SE(
        .data_i(data_SEi),
        .data_o(data_SEo)
        );

MUX_2to1 #(.size(32)) Mux_ALUSrc(
        .data0_i(data0_ALUSi),
        .data1_i(data1_ALUSi),
        .select_i(select_ALUSi),
        .data_o(data_ALUSo)
        );	
		
ALU ALU(
        .src1_i(src1_ALUi),
	    .src2_i(src2_ALUi),
	    .ctrl_i(ctrl_ALUi),
	    .result_o(result_ALUo),
		.zero_o(zero_ALUo)
	    );
		
Adder Adder2(
        .src1_i(Adder2_i1),     
	    .src2_i(Adder2_i2),     
	    .sum_o(Adder2_o)      
	    );
		
Shift_Left_Two_32 Shifter(
        .data_i(data_SL2i),
        .data_o(data_SL2o)
        ); 		
		
MUX_2to1 #(.size(32)) Mux_PC_Source(
        .data0_i(data0_PCSi),
        .data1_i(data1_PCSi),
        .select_i(select_PCSi),
        .data_o(data_PCSo)
        );	

MUX_2to1 #(.size(32)) Mux_ALUSrc2(
        .data0_i(data0_ALUSi2),
        .data1_i(data1_ALUSi2),
        .select_i(select_ALUSi2),
        .data_o(data_ALUS2o)
        );	

endmodule
		  


