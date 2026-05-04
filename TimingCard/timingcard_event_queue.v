module timingcard_event_queue #(
    parameter [15:0]  MAX_EVENTS = 16'd48,
    parameter integer INDEX_WIDTH = 6,
    parameter integer TIMER_WIDTH = 48
) (
    input  wire                     clk,
    input  wire                     clear,
    input  wire                     write_time_lo,
    input  wire                     write_time_hi,
    input  wire                     push,
    input  wire [31:0]              write_data,
    input  wire [INDEX_WIDTH-1:0]   head_index,
    output wire [TIMER_WIDTH-1:0]   head_delta_ticks,
    output reg  [31:0]              stage_time_lo,
    output reg  [31:0]              stage_time_hi,
    output reg  [INDEX_WIDTH-1:0]   count,
    output reg                      push_seen,
    output reg  [31:0]              last_time_lo,
    output reg  [31:0]              last_time_hi
);

    localparam [INDEX_WIDTH-1:0] MAX_COUNT = MAX_EVENTS[INDEX_WIDTH-1:0];

    reg [TIMER_WIDTH-1:0] event_delta_ticks [0:MAX_EVENTS-1];

    wire [63:0] stage_time_us;
    wire [63:0] last_time_us;

    assign stage_time_us = {stage_time_hi, stage_time_lo};
    assign last_time_us = {last_time_hi, last_time_lo};
    assign head_delta_ticks = (head_index < count) ?
                              event_delta_ticks[head_index] :
                              {TIMER_WIDTH{1'b0}};

    function [TIMER_WIDTH-1:0] delta_ticks_from_us;
        input [63:0] next_time_us;
        input [63:0] prev_time_us;
        reg   [63:0] delta_us;
        reg   [66:0] ticks;
        begin
            delta_us = next_time_us - prev_time_us;
            ticks = ({3'd0, delta_us} << 3) + ({3'd0, delta_us} << 1);
            delta_ticks_from_us = ticks[TIMER_WIDTH-1:0];
        end
    endfunction

    initial begin
        stage_time_lo = 32'd0;
        stage_time_hi = 32'd0;
        count = {INDEX_WIDTH{1'b0}};
        push_seen = 1'b0;
        last_time_lo = 32'd0;
        last_time_hi = 32'd0;
    end

    always @(posedge clk) begin
        if (clear) begin
            stage_time_lo <= 32'd0;
            stage_time_hi <= 32'd0;
            count <= {INDEX_WIDTH{1'b0}};
            push_seen <= 1'b0;
            last_time_lo <= 32'd0;
            last_time_hi <= 32'd0;
        end else begin
            if (write_time_lo) begin
                stage_time_lo <= write_data;
            end

            if (write_time_hi) begin
                stage_time_hi <= write_data;
            end

            if (push && (count < MAX_COUNT)) begin
                event_delta_ticks[count] <= delta_ticks_from_us(stage_time_us, last_time_us);
                count <= count + {{INDEX_WIDTH-1{1'b0}}, 1'b1};
                push_seen <= 1'b1;
                last_time_lo <= stage_time_lo;
                last_time_hi <= stage_time_hi;
            end
        end
    end

endmodule
