module timingcard_spi_decoder (
    input  wire        spi_cs_n,
    input  wire        spi_mosi,
    output wire        spi_miso,
    input  wire        spi_sck,
    output reg  [15:0] req_addr,
    output reg  [31:0] req_wdata,
    output reg         req_write_strobe,
    input  wire [7:0]  card_type,
    input  wire [7:0]  fw_major,
    input  wire [7:0]  fw_minor,
    input  wire [15:0] capabilities,
    input  wire [7:0]  max_local_events,
    input  wire [31:0] reg_read_data,
    input  wire        reg_read_valid,
    input  wire        reg_write_valid
);

    localparam PROTO_SOF        = 8'hC3;
    localparam CMD_IDENTIFY     = 8'h01;
    localparam CMD_READ_REG     = 8'h02;
    localparam CMD_WRITE_REG    = 8'h03;
    localparam ACK_OK           = 8'h79;
    localparam ACK_ERR          = 8'h1F;
    localparam STATUS_OK        = 8'h00;
    localparam STATUS_BAD_CMD   = 8'h01;
    localparam STATUS_BAD_LEN   = 8'h02;
    localparam STATUS_BAD_CRC   = 8'h03;
    localparam STATUS_BAD_ADDR  = 8'h04;
    localparam [1:0] FRAME_STATUS_OK      = 2'd0;
    localparam [1:0] FRAME_STATUS_BAD_CMD = 2'd1;
    localparam [1:0] FRAME_STATUS_BAD_LEN = 2'd2;
    localparam [1:0] FRAME_STATUS_BAD_CRC = 2'd3;
    localparam RX_FRAME_BYTES   = 80;
    localparam RESP_FRAME_BYTES = 20;
    localparam RX_STORE_BYTES   = 12;

    integer i;

    reg [7:0] rx_store [0:RX_STORE_BYTES-1];

    reg [7:0] rx_shift;
    reg [6:0] rx_byte_index;
    reg [2:0] rx_bit_index;
    reg [15:0] rx_payload_len;
    reg [15:0] rx_crc_calc;

    reg [4:0] tx_byte_index;
    reg [2:0] tx_shift_count;
    reg [7:0] tx_shift;
    reg       tx_loaded;

    reg       response_phase;
    reg       write_commit_pending;

    reg [7:0]  parsed_command_id;
    reg [15:0] parsed_payload_len;
    reg [1:0]  parsed_status;

    wire [7:0]  frame_command_id;
    wire [15:0] frame_payload_len;
    reg  [1:0]  frame_status;
    reg  [15:0] frame_req_addr;
    reg  [31:0] frame_req_wdata;
    reg         frame_write_commit_pending;
    reg  [15:0] frame_crc_rx;

    reg [7:0]  resp_ack;
    reg [7:0]  resp_status;
    reg [15:0] resp_payload_len;
    reg [7:0]  resp_payload0;
    reg [7:0]  resp_payload1;
    reg [7:0]  resp_payload2;
    reg [7:0]  resp_payload3;
    reg [7:0]  resp_payload4;
    reg [7:0]  resp_payload5;
    reg [15:0] resp_crc;

    reg [7:0] tx_byte_value;
    reg [7:0] tx_next_byte_value;

    wire [7:0] rx_next_byte;

    assign frame_command_id = rx_store[1];
    assign frame_payload_len = {rx_store[3], rx_store[2]};
    assign rx_next_byte = {rx_shift[6:0], spi_mosi};

    function [15:0] crc16_update;
        input [15:0] crc_in;
        input [7:0] data_byte;
        integer bit_idx;
        reg [15:0] crc_tmp;
        begin
            crc_tmp = crc_in ^ {data_byte, 8'h00};
            for (bit_idx = 0; bit_idx < 8; bit_idx = bit_idx + 1) begin
                if (crc_tmp[15]) begin
                    crc_tmp = (crc_tmp << 1) ^ 16'h1021;
                end else begin
                    crc_tmp = (crc_tmp << 1);
                end
            end
            crc16_update = crc_tmp;
        end
    endfunction

    function [7:0] response_byte;
        input [4:0]  byte_index;
        input [7:0]  ack;
        input [7:0]  status;
        input [15:0] payload_len;
        input [7:0]  payload0;
        input [7:0]  payload1;
        input [7:0]  payload2;
        input [7:0]  payload3;
        input [7:0]  payload4;
        input [7:0]  payload5;
        input [15:0] crc_value;
        begin
            response_byte = 8'd0;

            case (byte_index)
                5'd0: response_byte = PROTO_SOF;
                5'd1: response_byte = ack;
                5'd2: response_byte = status;
                5'd3: response_byte = payload_len[7:0];
                5'd4: response_byte = payload_len[15:8];
                default: begin
                    if (byte_index < (5'd5 + payload_len)) begin
                        case (byte_index - 5'd5)
                            5'd0: response_byte = payload0;
                            5'd1: response_byte = payload1;
                            5'd2: response_byte = payload2;
                            5'd3: response_byte = payload3;
                            5'd4: response_byte = payload4;
                            5'd5: response_byte = payload5;
                            default: response_byte = 8'd0;
                        endcase
                    end else if (byte_index == (5'd5 + payload_len)) begin
                        response_byte = crc_value[7:0];
                    end else if (byte_index == (5'd6 + payload_len)) begin
                        response_byte = crc_value[15:8];
                    end
                end
            endcase
        end
    endfunction

    function [7:0] frame_status_byte;
        input [1:0] status_code;
        begin
            case (status_code)
                FRAME_STATUS_OK:      frame_status_byte = STATUS_OK;
                FRAME_STATUS_BAD_CMD: frame_status_byte = STATUS_BAD_CMD;
                FRAME_STATUS_BAD_LEN: frame_status_byte = STATUS_BAD_LEN;
                FRAME_STATUS_BAD_CRC: frame_status_byte = STATUS_BAD_CRC;
                default:              frame_status_byte = STATUS_BAD_CMD;
            endcase
        end
    endfunction

    always @* begin
        case (frame_payload_len)
            16'd0: frame_crc_rx = {rx_store[5],  rx_store[4]};
            16'd1: frame_crc_rx = {rx_store[6],  rx_store[5]};
            16'd2: frame_crc_rx = {rx_store[7],  rx_store[6]};
            16'd3: frame_crc_rx = {rx_store[8],  rx_store[7]};
            16'd4: frame_crc_rx = {rx_store[9],  rx_store[8]};
            16'd5: frame_crc_rx = {rx_store[10], rx_store[9]};
            16'd6: frame_crc_rx = {rx_store[11], rx_store[10]};
            default: frame_crc_rx = 16'd0;
        endcase
    end

    always @* begin
        frame_status = FRAME_STATUS_OK;
        frame_req_addr = 16'd0;
        frame_req_wdata = 32'd0;
        frame_write_commit_pending = 1'b0;

        if (rx_store[0] != PROTO_SOF) begin
            frame_status = FRAME_STATUS_BAD_CMD;
        end else if (frame_payload_len > 16'd6) begin
            frame_status = FRAME_STATUS_BAD_LEN;
        end else if (frame_crc_rx != rx_crc_calc) begin
            frame_status = FRAME_STATUS_BAD_CRC;
        end else begin
            case (frame_command_id)
                CMD_IDENTIFY: begin
                    if (frame_payload_len != 16'd0) begin
                        frame_status = FRAME_STATUS_BAD_LEN;
                    end
                end

                CMD_READ_REG: begin
                    if (frame_payload_len == 16'd2) begin
                        frame_req_addr = {rx_store[5], rx_store[4]};
                    end else begin
                        frame_status = FRAME_STATUS_BAD_LEN;
                    end
                end

                CMD_WRITE_REG: begin
                    if (frame_payload_len == 16'd6) begin
                        frame_req_addr = {rx_store[5], rx_store[4]};
                        frame_req_wdata = {rx_store[9], rx_store[8], rx_store[7], rx_store[6]};
                        frame_write_commit_pending = 1'b1;
                    end else begin
                        frame_status = FRAME_STATUS_BAD_LEN;
                    end
                end

                default: begin
                    frame_status = FRAME_STATUS_BAD_CMD;
                end
            endcase
        end
    end

    always @* begin
        resp_ack = ACK_ERR;
        resp_status = frame_status_byte(parsed_status);
        resp_payload_len = 16'd0;
        resp_payload0 = 8'd0;
        resp_payload1 = 8'd0;
        resp_payload2 = 8'd0;
        resp_payload3 = 8'd0;
        resp_payload4 = 8'd0;
        resp_payload5 = 8'd0;

        if (parsed_status == FRAME_STATUS_OK) begin
            case (parsed_command_id)
                CMD_IDENTIFY: begin
                    if (parsed_payload_len == 16'd0) begin
                        resp_ack = ACK_OK;
                        resp_payload_len = 16'd6;
                        resp_payload0 = card_type;
                        resp_payload1 = fw_major;
                        resp_payload2 = fw_minor;
                        resp_payload3 = capabilities[7:0];
                        resp_payload4 = capabilities[15:8];
                        resp_payload5 = max_local_events;
                    end else begin
                        resp_status = STATUS_BAD_LEN;
                    end
                end

                CMD_READ_REG: begin
                    if (parsed_payload_len != 16'd2) begin
                        resp_status = STATUS_BAD_LEN;
                    end else if (!reg_read_valid) begin
                        resp_status = STATUS_BAD_ADDR;
                    end else begin
                        resp_ack = ACK_OK;
                        resp_payload_len = 16'd4;
                        resp_payload0 = reg_read_data[7:0];
                        resp_payload1 = reg_read_data[15:8];
                        resp_payload2 = reg_read_data[23:16];
                        resp_payload3 = reg_read_data[31:24];
                    end
                end

                CMD_WRITE_REG: begin
                    if (parsed_payload_len != 16'd6) begin
                        resp_status = STATUS_BAD_LEN;
                    end else if (!reg_write_valid) begin
                        resp_status = STATUS_BAD_ADDR;
                    end else begin
                        resp_ack = ACK_OK;
                    end
                end

                default: begin
                    resp_status = STATUS_BAD_CMD;
                end
            endcase
        end
    end

    always @* begin
        resp_crc = 16'hFFFF;
        resp_crc = crc16_update(resp_crc, resp_ack);
        resp_crc = crc16_update(resp_crc, resp_status);
        resp_crc = crc16_update(resp_crc, resp_payload_len[7:0]);
        resp_crc = crc16_update(resp_crc, resp_payload_len[15:8]);
        if (resp_payload_len > 16'd0) resp_crc = crc16_update(resp_crc, resp_payload0);
        if (resp_payload_len > 16'd1) resp_crc = crc16_update(resp_crc, resp_payload1);
        if (resp_payload_len > 16'd2) resp_crc = crc16_update(resp_crc, resp_payload2);
        if (resp_payload_len > 16'd3) resp_crc = crc16_update(resp_crc, resp_payload3);
        if (resp_payload_len > 16'd4) resp_crc = crc16_update(resp_crc, resp_payload4);
        if (resp_payload_len > 16'd5) resp_crc = crc16_update(resp_crc, resp_payload5);
    end

    always @* begin
        tx_byte_value = response_byte(tx_byte_index,
                                      resp_ack,
                                      resp_status,
                                      resp_payload_len,
                                      resp_payload0,
                                      resp_payload1,
                                      resp_payload2,
                                      resp_payload3,
                                      resp_payload4,
                                      resp_payload5,
                                      resp_crc);
        tx_next_byte_value = response_byte(tx_byte_index + 5'd1,
                                           resp_ack,
                                           resp_status,
                                           resp_payload_len,
                                           resp_payload0,
                                           resp_payload1,
                                           resp_payload2,
                                           resp_payload3,
                                           resp_payload4,
                                           resp_payload5,
                                           resp_crc);
    end

    assign spi_miso = (!spi_cs_n && response_phase) ? tx_shift[7] : 1'b0;

    initial begin
        rx_shift = 8'd0;
        rx_byte_index = 7'd0;
        rx_bit_index = 3'd0;
        rx_payload_len = 16'd0;
        rx_crc_calc = 16'hFFFF;
        tx_byte_index = 5'd0;
        tx_shift_count = 3'd0;
        tx_shift = 8'd0;
        tx_loaded = 1'b0;
        response_phase = 1'b0;
        write_commit_pending = 1'b0;
        parsed_command_id = 8'd0;
        parsed_payload_len = 16'd0;
        parsed_status = FRAME_STATUS_BAD_CMD;
        req_addr = 16'd0;
        req_wdata = 32'd0;
        req_write_strobe = 1'b0;

        for (i = 0; i < RX_STORE_BYTES; i = i + 1) begin
            rx_store[i] = 8'd0;
        end
    end

    always @(posedge spi_sck or posedge spi_cs_n) begin
        if (spi_cs_n) begin
            response_phase <= 1'b0;
            write_commit_pending <= 1'b0;
            req_write_strobe <= 1'b0;
            rx_shift <= 8'd0;
            rx_byte_index <= 7'd0;
            rx_bit_index <= 3'd0;
            rx_payload_len <= 16'd0;
            rx_crc_calc <= 16'hFFFF;
        end else begin
            req_write_strobe <= 1'b0;

            if (!response_phase) begin
                rx_shift <= rx_next_byte;

                if (rx_bit_index == 3'd7) begin
                    if (rx_byte_index < RX_STORE_BYTES) begin
                        rx_store[rx_byte_index] <= rx_next_byte;
                    end

                    if (rx_byte_index == 7'd0) begin
                        rx_crc_calc <= 16'hFFFF;
                    end else if (rx_byte_index == 7'd1) begin
                        rx_crc_calc <= crc16_update(16'hFFFF, rx_next_byte);
                    end else if (rx_byte_index == 7'd2) begin
                        rx_payload_len[7:0] <= rx_next_byte;
                        rx_crc_calc <= crc16_update(rx_crc_calc, rx_next_byte);
                    end else if (rx_byte_index == 7'd3) begin
                        rx_payload_len[15:8] <= rx_next_byte;
                        rx_crc_calc <= crc16_update(rx_crc_calc, rx_next_byte);
                    end else if ({9'd0, rx_byte_index} < (16'd4 + rx_payload_len)) begin
                        rx_crc_calc <= crc16_update(rx_crc_calc, rx_next_byte);
                    end

                    rx_bit_index <= 3'd0;

                    if (rx_byte_index == (RX_FRAME_BYTES - 1)) begin
                        response_phase <= 1'b1;
                        parsed_command_id <= frame_command_id;
                        parsed_payload_len <= frame_payload_len;
                        parsed_status <= frame_status;
                        req_addr <= frame_req_addr;
                        req_wdata <= frame_req_wdata;
                        write_commit_pending <= frame_write_commit_pending;
                    end else begin
                        rx_byte_index <= rx_byte_index + 7'd1;
                    end
                end else begin
                    rx_bit_index <= rx_bit_index + 3'd1;
                end
            end else if (write_commit_pending) begin
                req_write_strobe <= 1'b1;
                write_commit_pending <= 1'b0;
            end
        end
    end

    always @(negedge spi_sck or posedge spi_cs_n) begin
        if (spi_cs_n) begin
            tx_byte_index <= 5'd0;
            tx_shift_count <= 3'd0;
            tx_shift <= 8'd0;
            tx_loaded <= 1'b0;
        end else if (response_phase) begin
            if (!tx_loaded) begin
                tx_byte_index <= 5'd0;
                tx_shift_count <= 3'd0;
                tx_shift <= tx_byte_value;
                tx_loaded <= 1'b1;
            end else if (tx_shift_count == 3'd7) begin
                tx_shift_count <= 3'd0;
                if (tx_byte_index < (RESP_FRAME_BYTES - 1)) begin
                    tx_byte_index <= tx_byte_index + 5'd1;
                    tx_shift <= tx_next_byte_value;
                end else begin
                    tx_shift <= 8'd0;
                end
            end else begin
                tx_shift <= {tx_shift[6:0], 1'b0};
                tx_shift_count <= tx_shift_count + 3'd1;
            end
        end else begin
            tx_byte_index <= 5'd0;
            tx_shift_count <= 3'd0;
            tx_shift <= 8'd0;
            tx_loaded <= 1'b0;
        end
    end

endmodule
