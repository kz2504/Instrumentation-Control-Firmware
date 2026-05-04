module timingcard_spi_write_cdc (
    input  wire        clk,
    input  wire [15:0] write_addr_async,
    input  wire [31:0] write_data_async,
    input  wire        write_toggle_async,
    output reg         cmd_valid,
    output reg  [15:0] cmd_addr,
    output reg  [31:0] cmd_data
);

    reg        write_toggle_meta;
    reg        write_toggle_sync;
    reg        write_toggle_seen;
    reg [15:0] write_addr_meta;
    reg [15:0] write_addr_sync;
    reg [31:0] write_data_meta;
    reg [31:0] write_data_sync;

    initial begin
        write_toggle_meta = 1'b0;
        write_toggle_sync = 1'b0;
        write_toggle_seen = 1'b0;
        write_addr_meta = 16'd0;
        write_addr_sync = 16'd0;
        write_data_meta = 32'd0;
        write_data_sync = 32'd0;
        cmd_valid = 1'b0;
        cmd_addr = 16'd0;
        cmd_data = 32'd0;
    end

    always @(posedge clk) begin
        write_toggle_meta <= write_toggle_async;
        write_toggle_sync <= write_toggle_meta;
        write_addr_meta <= write_addr_async;
        write_addr_sync <= write_addr_meta;
        write_data_meta <= write_data_async;
        write_data_sync <= write_data_meta;

        cmd_valid <= 1'b0;
        if (write_toggle_sync != write_toggle_seen) begin
            write_toggle_seen <= write_toggle_sync;
            cmd_valid <= 1'b1;
            cmd_addr <= write_addr_sync;
            cmd_data <= write_data_sync;
        end
    end

endmodule
