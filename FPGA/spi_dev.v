//==============================================================================================
// SPI IP
// SPI Verilog Implementation (spi.v)
// Deborah Jahaj and Nathan Fusselman

//==============================================================================================
// Hardware Target

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_[031-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//==============================================================================================

module spi_dev (
		input  wire        clk,        //    clk.clk
		input  wire        reset,      //  reset.reset
		input  wire [1:0]  address,    // avalon.address
		input  wire [3:0]  byteenable, //       .byteenable
		input  wire        chipselect, //       .chipselect
		input  wire        read,       //       .read
		output reg  [31:0] readdata,   //       .readdata
		input  wire        write,      //       .write
		input  wire [31:0] writedata,  //       .writedata
		output wire        sclk,       //   port.sclk
		output wire        tx,         //       .tx
		input  wire        rx,         //       .rx
		output wire        cs0,        //       .cs0
		output wire        cs1,        //       .cs1
		output wire        cs2,        //       .cs2
		output wire        cs3         //       .cs3
	);

	 // internal    
    wire [31:0] status;
    reg [31:0] control;
    reg [31:0] brd;
	 wire [31:0] RX_data_out;
	 wire [31:0] RX_data_in;
	 wire [31:0] TX_data; 
	 
	 wire TX_FIFO_WRITE;
	 wire TX_FIFO_READ;
	 wire RX_FIFO_WRITE;
	 wire RX_FIFO_READ;
	 wire TX_CLEAR_OV;
	 wire TX_RESET;
	 wire BAUD_CLOCK;
	 wire CS_ASSERT;
	 wire SEL_CS_AUTO, SEL_CS_ENABLE, SEL_CS;
	 wire [1:0] SEL_MODE;
	 wire [3:0] CS;
	
	 // Register Map
    // ofs  fn
    //   0  data 	  (r/w)
    //   4  status  (r/w1c)
    //   8  control (r/w)
    //  12  brd     (r/w)
    
    // Register Numbers
    parameter DATA_REG     = 2'b00;
    parameter STATUS_REG   = 2'b01;
    parameter CONTROL_REG  = 2'b10;
    parameter BRD_REG      = 2'b11;

	 // Read Register
    always @ (*)
    begin
        if (read & chipselect)
            case (address)
                DATA_REG: 
                    readdata = RX_data_out;
                STATUS_REG:
                    readdata = status;
                CONTROL_REG: 
                    readdata = control;
                BRD_REG: 
                    readdata = brd;
            endcase
        else
            readdata = 32'b0;
    end        

    // Write Register
    always @ (posedge clk or posedge reset)
    begin
        if (reset)
        begin
				control[4:0] 	<= 5'b11111;	  // WORD_SIZE
				control[8:5] 	<= 4'b1111;		  // CS0-3_AUTO
				control[12:9] 	<= 4'b0000;		  // CS0_3_ENABLE
				control[14:13] <= 2'b00;		  // CS_SELECT
				control[15] 	<= 1'b1;			  // ENABLE
				control[17:16] <= 2'b00;		  // MODE0
				control[19:18] <= 2'b00;		  // MODE1
				control[21:20] <= 2'b00;		  // MODE2
				control[23:22] <= 2'b00;		  // MODE3
				control[31:24] <= 2'b00;		  // MODE4				
				brd[31:0]		<= 32'h00000280; // 5MHz
        end
        else
        begin
            if (write & chipselect)
            begin
                case (address)
                    CONTROL_REG: 
                        control <= writedata;
                    BRD_REG: 
                        brd <= writedata;
                endcase
            end
        end
    end
	
	assign cs0 = ~CS[0];
	assign cs1 = ~CS[1];
	assign cs2 = ~CS[2];
	assign cs3 = ~CS[3];
	
	assign RX_FIFO_READ = read & chipselect & (address == DATA_REG);
	assign TX_FIFO_WRITE = write & chipselect & (address == DATA_REG);
	assign TX_CLEAR_OV = write & chipselect & (address == STATUS_REG) & writedata[3];
	assign RX_CLEAR_OV = write & chipselect & (address == STATUS_REG) & writedata[0];
	assign TX_RESET = write & chipselect & (address == STATUS_REG) & writedata[7];
	assign RX_RESET = write & chipselect & (address == STATUS_REG) & writedata[6];
	
	clock_generator clock_generator (.clk(clk), .reset(reset), 
												.enable(control[15]), .brd(brd), .baud_out(BAUD_CLOCK));
	
	edge_triggered_FIFO TX_FIFO(.Read(TX_FIFO_READ),
									    .Write(TX_FIFO_WRITE),
									    .ClearOV(TX_CLEAR_OV),
									    .Clock(clk), .Reset(reset|TX_RESET),
								       .DataIn(writedata), .DataOut(TX_data), .store_count(status[15:12]),
								       .Full(status[4]), .Empty(status[5]), .OV(status[3]));
									 
	edge_triggered_FIFO RX_FIFO(.Read(RX_FIFO_READ),
									    .Write(RX_FIFO_WRITE),
									    .ClearOV(RX_CLEAR_OV),
									    .Clock(clk), .Reset(reset|RX_RESET),
								       .DataIn(RX_data_in), .DataOut(RX_data_out), .store_count(status[11:8]),
								       .Full(status[1]), .Empty(status[2]), .OV(status[0]));
	
	cs_sclk_manager manager(.SCLK_IN(BAUD_CLOCK),
									.SCLK_ENABLE(TX_FIFO_READ),
									.CS_ASSERT(CS_ASSERT),
									.MODE(control[23:16]),
									.CS_SELECT(control[14:13]),
									.CS_AUTO(control[8:5]),
									.CS_ENABLE(control[12:9]),
									.SEL_CS_AUTO(SEL_CS_AUTO),
									.SEL_CS_ENABLE(SEL_CS_ENABLE),
									.SEL_CS(SEL_CS),
									.SEL_MODE(SEL_MODE),
									.CS(CS),
									.SCLK_OUT(sclk)
									);
									

	serializer TX_RX_serializer(.CLK(clk),
										 .SCLK((~BAUD_CLOCK & (SEL_MODE[1] ^ SEL_MODE[0])) | (BAUD_CLOCK & ~(SEL_MODE[1] ^ SEL_MODE[0]))),
									    .RESET(reset),
									    .SEND(~status[5]),
									    .CS_AUTO(SEL_CS_AUTO),
									    .CS_ENABLE(SEL_CS_ENABLE),
									    .MODE(SEL_MODE),
									    .WORD_SIZE(control[4:0]),
										 .RX(rx),
										 .RX_FIFO_WRITE(RX_FIFO_WRITE),
										 .DATA_OUT(RX_data_in),
										 .TX(tx),
									    .TX_FIFO_READ(TX_FIFO_READ),
										 .DATA_IN(TX_data),
									    .CS_ASSERT(CS_ASSERT)
									    );
									 
endmodule
	

//==============================================================================================

// Baud Rate Generator
module clock_generator(
	input clk, reset, enable,
	input[31:0] brd,
	output reg baud_out
	);
		
	// brd[31:6] is the integer part, brd[5:0] is the fractional part
	// since toggling about at 2x rate
	// toggling integer count is brd[31:7] and fractional count is brd[6:0]
	
	reg clk_div_2_out;
	reg[31:0] count;
	reg[31:0] target; 

	always @ (posedge clk)
	begin
		if(reset || !enable)
		begin
			clk_div_2_out <= 1'b0;
			baud_out <= 1'b0;
			count <= 32'b0;
			target <= brd;
		end
		else
		begin
			//toggle output
			clk_div_2_out <= !clk_div_2_out;
			//increment by 1.0
			count <= count + 32'b10000000;
			if (count[31:7] == target[31:7])
			begin
				 target <= target + brd;
				 baud_out <= !baud_out;
			end
		end
	end

endmodule

//==============================================================================================

module edge_detect(
	input signal_in, clock,
	output reg signal_out
	);

	reg previous_state;
	
	always @ (posedge clock)
	begin
		if((signal_in != previous_state) & signal_in)
			signal_out = 1;	// on a rising edge
		else
			signal_out = 0;	// on a level
		
		previous_state = signal_in;
		
	end

endmodule

//==============================================================================================

module edge_triggered_FIFO(
	input  Read, Write, Clock, Reset, ClearOV,
	input  [31:0] DataIn,
	output [31:0] DataOut,
	output [3:0] store_count,			
	output Full, Empty, OV);
	
	wire read_edge, write_edge, clearOV_edge;
	
	edge_detect read_edge_detect(.signal_in(Read), .clock(Clock),
										  .signal_out(read_edge));
										  
	edge_detect write_edge_detect(.signal_in(Write), .clock(Clock),
											.signal_out(write_edge));
											 
	edge_detect clearOV_edge_detect(.signal_in(ClearOV), .clock(Clock),
											  .signal_out(clearOV_edge));
	
	FIFO FIFO(.Read(read_edge), .Write(write_edge), 
				 .Clock(Clock), .Reset(Reset), .ClearOV(clearOV_edge),
				 .DataIn(DataIn), .DataOut(DataOut), .store_count(store_count),
				 .Full(Full), .Empty(Empty), .OV(OV));
						  
endmodule


//==============================================================================================

module FIFO(
	input  Read, Write, Clock, Reset, ClearOV,
	input  [31:0] DataIn,
	output wire [31:0] DataOut,
	output reg [3:0] store_count,
	output wire Full, Empty, OV
	);
	
	reg [31:0] Stack [15:0]; //Storage array
	reg [1:0] state;
	reg [3:0] ReadPtr, WritePtr;
	parameter EMPTY = 2'b00, NEUTRAL = 2'b01, FULL = 2'b10, OVR = 2'b11;
	
	assign Empty = state==EMPTY;
	assign Full = (state==FULL)|(state==OVR);
	assign OV = state==OVR;
	assign DataOut = Stack[ReadPtr];
	
	always @ (posedge Clock, posedge Reset)
	begin 
		if(Reset)
		begin
			state = EMPTY;
			ReadPtr = 4'd0; WritePtr = 4'd0; store_count = 4'd0;
		end
		else
		begin
			case(state)
				EMPTY:
				begin
					if(Read)  begin state = EMPTY; end																												  //1
					if(Write) begin state = NEUTRAL; Stack[WritePtr]=DataIn; WritePtr = WritePtr+1'b1; store_count = store_count+1'b1; end //2
				end
				NEUTRAL:
				begin
					if(Read  && (store_count == 1))  begin state = EMPTY; ReadPtr = ReadPtr+1'b1; store_count = store_count-1'b1; end 									 //3
					if(Write && (store_count < 14))  begin state = NEUTRAL; Stack[WritePtr]=DataIn; WritePtr = WritePtr+1'b1; store_count = store_count+1'b1; end //4
					if(Read  && (store_count > 1))   begin state = NEUTRAL; ReadPtr  = ReadPtr+1'b1; store_count = store_count-1'b1; end 								 //5
					if(Write && (store_count == 14)) begin state = FULL; Stack[WritePtr]=DataIn; WritePtr = WritePtr+1'b1; store_count = store_count+1'b1; end    //6
				end
				FULL: 
				begin
					if(Read)  begin state = NEUTRAL; ReadPtr = ReadPtr+1'b1; store_count = store_count-1'b1; end //7
					if(Write) begin state = OVR; end																				   //8
					
				end
				OVR: 
				begin
					if(Read || Write) begin state = OVR; end  // 9
					if(ClearOV) 		begin state = FULL; end //10
				end
			endcase
		end
	end
	
endmodule

//==============================================================================================

// ADD MODE SUPPORT
module cs_sclk_manager(
	input SCLK_IN,
	input SCLK_ENABLE,
	input CS_ASSERT,
	input [7:0] MODE,
	input [1:0] CS_SELECT,
	input [3:0] CS_AUTO, CS_ENABLE,
	output reg SEL_CS_AUTO, SEL_CS_ENABLE, SEL_CS,
	output reg [1:0] SEL_MODE,
	output reg [3:0] CS,
	output SCLK_OUT
	);

	parameter DEV0 = 2'b00, DEV1 = 2'b01, DEV2 = 2'b10, DEV3 = 2'b11;

	integer i;
	
	always @ (*)
	begin
		for (i = 0; i < 4; i = i + 1)
		begin 
			CS[i] = (CS_ENABLE[i] & ~CS_AUTO[i]) | (CS_AUTO[i] & CS_ASSERT & (i == CS_SELECT));
		end
		
		case(CS_SELECT)
			DEV0: begin SEL_CS_AUTO = CS_AUTO[0]; SEL_CS_ENABLE = CS_ENABLE[0]; SEL_CS = CS[0]; SEL_MODE = MODE[1:0]; end
			DEV1: begin SEL_CS_AUTO = CS_AUTO[1]; SEL_CS_ENABLE = CS_ENABLE[1]; SEL_CS = CS[1]; SEL_MODE = MODE[3:2]; end
			DEV2: begin SEL_CS_AUTO = CS_AUTO[2]; SEL_CS_ENABLE = CS_ENABLE[2]; SEL_CS = CS[2]; SEL_MODE = MODE[5:4]; end
			DEV3: begin SEL_CS_AUTO = CS_AUTO[3]; SEL_CS_ENABLE = CS_ENABLE[3]; SEL_CS = CS[3]; SEL_MODE = MODE[7:6]; end	
		endcase

	end
	
	//assign SCLK_OUT = SCLK_IN & SCLK_ENABLE;
	assign SCLK_OUT = (((SCLK_IN & SCLK_ENABLE) & ~SEL_MODE[1]) | (~(SCLK_IN & SCLK_ENABLE) & SEL_MODE[1]));

endmodule

//==============================================================================================

module serializer(
	input CLK, SCLK, RESET, SEND,
	input CS_AUTO, CS_ENABLE,
	input [1:0] MODE,
	input [4:0] WORD_SIZE,
	input [31:0] DATA_IN,
	input RX,
	output RX_FIFO_WRITE,
	output reg [31:0] DATA_OUT,
	output reg TX_FIFO_READ,
	output reg TX,
	output reg CS_ASSERT
	);

	reg [4:0] count;
	reg [31:0] latch_data;
	reg [1:0] state;
	reg last_sclk;
	parameter IDLE_STATE = 2'b00, CS_ASSERT_STATE = 2'b01, TX_RX_STATE = 2'b10;
	
	assign RX_FIFO_WRITE = ~TX_FIFO_READ;
	
	always @ (posedge CLK)
	begin
		if(RESET)
		begin
		state = IDLE_STATE;
		end
		else
		begin
			if (SCLK != last_sclk)
			begin
				if (SCLK)
				begin
					case(state)
						IDLE_STATE:
						begin
							TX_FIFO_READ = 0; count = WORD_SIZE; CS_ASSERT = 0; latch_data = DATA_IN;
							if(SEND & CS_AUTO) begin state = CS_ASSERT_STATE; end
							if(SEND & ~CS_AUTO) begin state = TX_RX_STATE; end
						end
						CS_ASSERT_STATE:
						begin
							TX_FIFO_READ = 0; count = WORD_SIZE; CS_ASSERT = 1;
							state = TX_RX_STATE;
						end
						TX_RX_STATE: 
						begin
							DATA_OUT[count] = RX; TX_FIFO_READ = 1; CS_ASSERT = 1;
							if(count == 0) begin state = IDLE_STATE; end
							else begin state = TX_RX_STATE; count = count-1'b1; end
						end
						default:
						begin
							state = IDLE_STATE;
						end
					endcase
				end
				else
				begin
					if(state == TX_RX_STATE) begin TX = latch_data[count]; end
						else begin TX = 0; end
				end
			end
		end
		last_sclk = SCLK;
	end

endmodule

//==============================================================================================