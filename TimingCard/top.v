`include "fpga_timing_regs.vh"

module top (
    input  wire        clk,

    output wire [15:0] out_5v,
    output wire [15:0] out_3v3,
    output wire        oe,
    output wire        sync,
    output wire        irq,

    input  wire        spi_cs_n,
    input  wire        spi_mosi,
    output wire        spi_miso,
    input  wire        spi_sck,

    output wire        fram_scl,
    output wire        fram_sda,

    output wire [8:0]  testpoint
);

    reg  [15:0] store_write_addr;
    reg  [31:0] store_write_data;
    reg         store_write_toggle;

    wire [15:0] req_addr;
    wire [31:0] req_wdata;
    wire        req_write_strobe;

    wire [31:0] store_read_data;
    wire        store_read_valid;
    wire        store_write_valid;
    wire [31:0] channel_level_bits;
    wire        sync_state;

    reg  [31:0] reg_read_data;
    reg         reg_read_valid;
    reg         reg_write_valid;

    initial begin
        store_write_addr = 16'd0;
        store_write_data = 32'd0;
        store_write_toggle = 1'b0;
    end

    timingcard_schedule_store u_store (
        .clk             (clk),
        .write_addr      (req_addr),
        .write_addr_async(store_write_addr),
        .write_data_async(store_write_data),
        .write_toggle_async(store_write_toggle),
        .read_addr       (req_addr),
        .read_data       (store_read_data),
        .read_valid      (store_read_valid),
        .write_valid     (store_write_valid),
        .channel_level_bits(channel_level_bits),
        .sync_state      (sync_state)
    );

    timingcard_spi_decoder u_spi_decoder (
        .spi_cs_n         (spi_cs_n),
        .spi_mosi         (spi_mosi),
        .spi_miso         (spi_miso),
        .spi_sck          (spi_sck),
        .req_addr         (req_addr),
        .req_wdata        (req_wdata),
        .req_write_strobe (req_write_strobe),
        .card_type        (`CARD_TYPE_FPGA),
        .fw_major         (`CARD_FW_MAJOR),
        .fw_minor         (`CARD_FW_MINOR),
        .capabilities     (`CARD_CAPABILITIES),
        .max_local_events (`CARD_MAX_LOCAL_EVENTS),
        .reg_read_data    (reg_read_data),
        .reg_read_valid   (reg_read_valid),
        .reg_write_valid  (reg_write_valid)
    );

    always @* begin
        reg_read_data = 32'd0;
        reg_read_valid = 1'b1;
        reg_write_valid = 1'b0;

        case (req_addr)
            `CARD_REG_ID: begin
                reg_read_data = `CARD_ID_MAGIC;
            end
            `CARD_REG_TYPE: begin
                reg_read_data = {24'd0, `CARD_TYPE_FPGA};
            end
            `CARD_REG_FW_VERSION: begin
                reg_read_data = {8'd0, `CARD_FW_MAJOR, 8'd0, `CARD_FW_MINOR};
            end
            `CARD_REG_CAPABILITIES: begin
                reg_read_data = {16'd0, `CARD_CAPABILITIES};
            end
            `CARD_REG_MAX_LOCAL_EVENTS: begin
                reg_read_data = {24'd0, `CARD_MAX_LOCAL_EVENTS};
            end
            default: begin
                reg_read_data = store_read_data;
                reg_read_valid = store_read_valid;
                reg_write_valid = store_write_valid;
            end
        endcase
    end

    always @(posedge spi_sck) begin
        if (req_write_strobe && store_write_valid) begin
            store_write_addr <= req_addr;
            store_write_data <= req_wdata;
            store_write_toggle <= ~store_write_toggle;
        end
    end

    assign out_5v = channel_level_bits[15:0];
    assign out_3v3 = channel_level_bits[31:16];
    assign oe = 1'b0;
    assign sync = sync_state;
    assign irq = 1'b0;

    assign fram_scl = 1'b1;
    assign fram_sda = 1'b1;
    assign testpoint = channel_level_bits[8:0];

endmodule
