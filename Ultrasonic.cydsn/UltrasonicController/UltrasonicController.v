
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
// Generated on 05/15/2016 at 22:10
// Component: UltrasonicController
module UltrasonicController (
	input wire microSecondClock,
	input wire ultrasonicClock,
    input wire trigger,
	input wire ultrasonicRX,
    input wire reset,
    
	output reg receivedInterrupt,
	output wire [15:0] timeMicroSeconds,
	output wire ultrasonicTX
);

//`#start body` -- edit after this line, do not edit this line


localparam True = 1'b1;
localparam False = 1'b0;
localparam transmissionCounterMax = 8'd20;

localparam returnTimeCounterMax = 16'd6061; // time for sound to travel two metres, in μs.

generate 

reg [7:0] transmissionCounter;
wire transmissionCounterRunning = (transmissionCounter > 0);

always @(posedge ultrasonicClock) begin
    if (reset) begin
        transmissionCounter <= 8'd0;
    end else if (trigger) begin
        transmissionCounter <= transmissionCounterMax;
    end else if (transmissionCounterRunning) begin
        transmissionCounter <= transmissionCounter - 8'd1;
    end else begin
        transmissionCounter <= 8'd0;
    end
end

assign ultrasonicTX = transmissionCounterRunning & ultrasonicClock;


reg [15:0] returnTimeCounter;
reg waitingOnRX;
wire returnTimeCounterRunning =
        (returnTimeCounter <  returnTimeCounterMax)
     && (returnTimeCounter != 16'd0);
     
reg [15:0] finalReturnTime;

always @(posedge microSecondClock) begin
    casex ({
        reset,
        trigger,
        ultrasonicRX && waitingOnRX,
        returnTimeCounterRunning
    })
        4'b1xxx: begin
            receivedInterrupt <= False;
            finalReturnTime <= 16'd0;
            returnTimeCounter <= 16'd0;
            waitingOnRX <= False;
        end
        4'b01xx: begin
            receivedInterrupt <= False;
            finalReturnTime <= finalReturnTime;
            returnTimeCounter <= 16'd1;
            waitingOnRX <= True;
        end
        4'b001x: begin
            receivedInterrupt <= False;
            finalReturnTime <= returnTimeCounter;
            returnTimeCounter <= 16'd0;
            waitingOnRX <= False;
        end
        4'b0001: begin
            receivedInterrupt <= True;
            finalReturnTime <= finalReturnTime;
            returnTimeCounter <= returnTimeCounter + 15'd1;
            waitingOnRX <= waitingOnRX;
        end
        4'b0000: begin
            receivedInterrupt <= False;
            finalReturnTime <= finalReturnTime;
            returnTimeCounter <= 16'd0;
            waitingOnRX <= False;
        end
        default: begin
            receivedInterrupt <= False;
            finalReturnTime <= finalReturnTime;
            returnTimeCounter <= 16'd0;
            waitingOnRX <= False;
        end
    endcase
end

assign timeMicroSeconds = finalReturnTime;

endgenerate

//`#end` -- edit above this line, do not edit this line
endmodule
//`#start footer` -- edit after this line, do not edit this line
//`#end` -- edit above this line, do not edit this line
