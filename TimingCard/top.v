module top (
    input  wire        clk,

    output wire [15:0] out_5v,
    output wire [15:0] out_3v3,
    output wire        oe,

    input  wire        spi_cs_n,
    input  wire        spi_mosi,
    output wire        spi_miso,
    input  wire        spi_sck,

    output wire        fram_scl,
    inout  wire        fram_sda,

    output wire [8:0]  testpoint
);

    // ============================================================
    // 10 MHz clock divider
    //
    // clk = 10 MHz
    // counter bit 23 toggles around 0.6 Hz
    // counter bit 22 toggles around 1.2 Hz
    // lower bits are faster
    // ============================================================

    reg [31:0] counter = 32'd0;

    always @(posedge clk) begin
        counter <= counter + 1'b1;
    end


    // ============================================================
    // Output enable
    //
    // IMPORTANT:
    // Change this if your external buffer OE is active-low.
    //
    // If OE active-high: assign oe = 1'b1;
    // If OE active-low:  assign oe = 1'b0;
    // ============================================================

    assign oe = 1'b1;


    // ============================================================
    // Simple visible/scopable patterns
    // ============================================================

    // Slow binary count on 5V buffered outputs
    assign out_5v = counter[27:12];

    // Inverted/slightly shifted count on 3.3V outputs
    assign out_3v3 = ~counter[26:11];

    // Testpoints get individual divided clocks
    assign testpoint[0] = counter[20];
    assign testpoint[1] = counter[21];
    assign testpoint[2] = counter[22];
    assign testpoint[3] = counter[23];
    assign testpoint[4] = counter[24];
    assign testpoint[5] = counter[25];
    assign testpoint[6] = counter[26];
    assign testpoint[7] = counter[27];
    assign testpoint[8] = counter[28];


    // ============================================================
    // Unused interfaces parked safely
    // ============================================================

    // Do not drive MISO unless you are intentionally testing SPI.
    // If MISO is on a shared bus, high-Z is safest.
    assign spi_miso = 1'bz;

    // Park FRAM I2C high-Z / inactive.
    // For real I2C these should be open-drain, but for this blink test
    // we are not using them.
    assign fram_scl = 1'b1;
    assign fram_sda = 1'bz;

endmodule   