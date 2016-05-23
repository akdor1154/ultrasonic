
//`#start header` -- edit after this line, do not edit this line
// ========================================
//
// Copyright YOUR COMPANY, THE YEAR
// All Rights Reserved
// UNPUBLISHED, LICENSED SOFTWARE.
//
// CONFIDENTIAL AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF your company.
//
// ========================================
`include "cypress.v"
//`#end` -- edit above this line, do not edit this line
// Generated on 05/23/2016 at 10:29
// Component: notCrapCounter
module notCrapCounter (
	input wire
        clock,
	    countEnable,
	    reset,
	output wire countComplete
);

reg [4:0] count;

always @(posedge clock) begin
    if (reset) begin
        count <= 5'b11111;
    end else if (~countComplete & countEnable) begin
        count <= count - 5'd1;
    end else begin
        count <= count;
    end
end

assign countComplete = count == 5'd0;



//`#start body` -- edit after this line, do not edit this line

//        Your code goes here

//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
