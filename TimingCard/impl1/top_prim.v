// Verilog netlist produced by program LSE :  version Diamond (64-bit) 3.14.0.75.2
// Netlist written on Tue Apr 28 01:14:48 2026
//
// Verilog Description of module top
//

module top (clk, out_5v, out_3v3, oe, spi_cs_n, spi_mosi, spi_miso, 
            spi_sck, fram_scl, fram_sda, testpoint) /* synthesis syn_module_defined=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(1[8:11])
    input clk;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(2[24:27])
    output [15:0]out_5v;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    output [15:0]out_3v3;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    output oe;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(6[24:26])
    input spi_cs_n;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(8[24:32])
    input spi_mosi;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(9[24:32])
    output spi_miso;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(10[24:32])
    input spi_sck;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(11[24:31])
    output fram_scl;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(13[24:32])
    output fram_sda /* synthesis .original_dir=IN_OUT */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(14[24:32])
    output [8:0]testpoint;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    
    wire clk_c /* synthesis SET_AS_NETWORK=clk_c, is_clock=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(2[24:27])
    
    wire GND_net, VCC_net, testpoint_c, testpoint_0, testpoint_1, 
        testpoint_2, testpoint_3, testpoint_4, testpoint_5, testpoint_6, 
        out_5v_c_7, out_5v_c_6, out_5v_c_5, out_5v_c_4, out_5v_c_3, 
        out_5v_c_2, out_5v_c_1, out_5v_c_0, out_3v3_c_15, out_3v3_c_14, 
        out_3v3_c_13, out_3v3_c_12, out_3v3_c_11, out_3v3_c_10, out_3v3_c_9, 
        out_3v3_c_8, out_3v3_c_7, out_3v3_c_6, out_3v3_c_5, out_3v3_c_4, 
        out_3v3_c_3, out_3v3_c_2, out_3v3_c_1, out_3v3_c_0, testpoint_c_8;
    wire [31:0]counter;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(28[16:23])
    
    wire n29, n28, n27, n26, n25, n24, n23, n22, n21, n20, 
        n19, n122, n123, n124, n125, n126, n127, n128, n129, 
        n130, n131, n132, n133, n134, n135, n136, n137, n138, 
        n139_adj_1, n140_adj_2, n141_adj_3, n142_adj_4, n143_adj_5, 
        n144_adj_6, n145_adj_7, n146_adj_8, n147_adj_9, n148_adj_10, 
        n149_adj_11, n150_adj_12, n141, n142, n143, n144, n145, 
        n146, n147, n148, n149, n150, n151, n152, n140, n139;
    
    VHI i2 (.Z(VCC_net));
    OB out_5v_pad_15 (.I(testpoint_c), .O(out_5v[15]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    LUT4 out_5v_14__I_0_i12_1_lut (.A(testpoint_4), .Z(out_3v3_c_11)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i12_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i11_1_lut (.A(testpoint_5), .Z(out_3v3_c_10)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i11_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i10_1_lut (.A(testpoint_6), .Z(out_3v3_c_9)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i10_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i9_1_lut (.A(out_5v_c_7), .Z(out_3v3_c_8)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i9_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i8_1_lut (.A(out_5v_c_6), .Z(out_3v3_c_7)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i8_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i7_1_lut (.A(out_5v_c_5), .Z(out_3v3_c_6)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i7_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i6_1_lut (.A(out_5v_c_4), .Z(out_3v3_c_5)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i6_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i5_1_lut (.A(out_5v_c_3), .Z(out_3v3_c_4)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i5_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i4_1_lut (.A(out_5v_c_2), .Z(out_3v3_c_3)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i4_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i3_1_lut (.A(out_5v_c_1), .Z(out_3v3_c_2)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i3_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i2_1_lut (.A(out_5v_c_0), .Z(out_3v3_c_1)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i2_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i1_1_lut (.A(counter[11]), .Z(out_3v3_c_0)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i1_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i16_1_lut (.A(testpoint_0), .Z(out_3v3_c_15)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i16_1_lut.init = 16'h5555;
    VLO i1 (.Z(GND_net));
    PUR PUR_INST (.PUR(VCC_net));
    defparam PUR_INST.RST_PULSE = 1;
    TSALL TSALL_INST (.TSALL(GND_net));
    OB out_5v_pad_14 (.I(testpoint_0), .O(out_5v[14]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    GSR GSR_INST (.GSR(VCC_net));
    LUT4 out_5v_14__I_0_i15_1_lut (.A(testpoint_1), .Z(out_3v3_c_14)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i15_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i14_1_lut (.A(testpoint_2), .Z(out_3v3_c_13)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i14_1_lut.init = 16'h5555;
    LUT4 out_5v_14__I_0_i13_1_lut (.A(testpoint_3), .Z(out_3v3_c_12)) /* synthesis lut_function=(!(A)) */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(56[22:37])
    defparam out_5v_14__I_0_i13_1_lut.init = 16'h5555;
    CCU2D counter_15_16_add_4_29 (.A0(testpoint_c), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(testpoint_c_8), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n152), .S0(n123), .S1(n122));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_29.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_29.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_29.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_29.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_27 (.A0(testpoint_1), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(testpoint_0), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n151), .COUT(n152), .S0(n125), .S1(n124));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_27.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_27.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_27.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_27.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_25 (.A0(testpoint_3), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(testpoint_2), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n150), .COUT(n151), .S0(n127), .S1(n126));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_25.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_25.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_25.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_25.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_23 (.A0(testpoint_5), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(testpoint_4), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n149), .COUT(n150), .S0(n129), .S1(n128));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_23.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_23.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_23.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_23.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_21 (.A0(out_5v_c_7), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(testpoint_6), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n148), .COUT(n149), .S0(n131), .S1(n130));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_21.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_21.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_21.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_21.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_19 (.A0(out_5v_c_5), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(out_5v_c_6), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n147), .COUT(n148), .S0(n133), .S1(n132));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_19.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_19.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_19.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_19.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_17 (.A0(out_5v_c_3), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(out_5v_c_4), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n146), .COUT(n147), .S0(n135), .S1(n134));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_17.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_17.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_17.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_17.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_15 (.A0(out_5v_c_1), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(out_5v_c_2), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n145), .COUT(n146), .S0(n137), .S1(n136));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_15.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_15.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_15.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_15.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_13 (.A0(counter[11]), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(out_5v_c_0), .B1(GND_net), .C1(GND_net), 
          .D1(GND_net), .CIN(n144), .COUT(n145), .S0(n139_adj_1), .S1(n138));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_13.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_13.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_13.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_13.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_11 (.A0(n20), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n19), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .CIN(n143), .COUT(n144), .S0(n141_adj_3), .S1(n140_adj_2));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_11.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_11.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_11.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_11.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_9 (.A0(n22), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n21), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .CIN(n142), .COUT(n143), .S0(n143_adj_5), .S1(n142_adj_4));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_9.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_9.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_9.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_9.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_7 (.A0(n24), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n23), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .CIN(n141), .COUT(n142), .S0(n145_adj_7), .S1(n144_adj_6));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_7.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_7.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_7.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_7.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_5 (.A0(n26), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n25), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .CIN(n140), .COUT(n141), .S0(n147_adj_9), .S1(n146_adj_8));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_5.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_5.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_5.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_5.INJECT1_1 = "NO";
    CCU2D counter_15_16_add_4_3 (.A0(n28), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n27), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .CIN(n139), .COUT(n140), .S0(n149_adj_11), .S1(n148_adj_10));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_3.INIT0 = 16'hfaaa;
    defparam counter_15_16_add_4_3.INIT1 = 16'hfaaa;
    defparam counter_15_16_add_4_3.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_3.INJECT1_1 = "NO";
    FD1S3AX counter_15_16__i28 (.D(n122), .CK(clk_c), .Q(testpoint_c_8)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i28.GSR = "ENABLED";
    FD1S3AX counter_15_16__i27 (.D(n123), .CK(clk_c), .Q(testpoint_c)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i27.GSR = "ENABLED";
    FD1S3AX counter_15_16__i26 (.D(n124), .CK(clk_c), .Q(testpoint_0)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i26.GSR = "ENABLED";
    FD1S3AX counter_15_16__i25 (.D(n125), .CK(clk_c), .Q(testpoint_1)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i25.GSR = "ENABLED";
    FD1S3AX counter_15_16__i24 (.D(n126), .CK(clk_c), .Q(testpoint_2)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i24.GSR = "ENABLED";
    FD1S3AX counter_15_16__i23 (.D(n127), .CK(clk_c), .Q(testpoint_3)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i23.GSR = "ENABLED";
    FD1S3AX counter_15_16__i22 (.D(n128), .CK(clk_c), .Q(testpoint_4)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i22.GSR = "ENABLED";
    FD1S3AX counter_15_16__i21 (.D(n129), .CK(clk_c), .Q(testpoint_5)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i21.GSR = "ENABLED";
    FD1S3AX counter_15_16__i20 (.D(n130), .CK(clk_c), .Q(testpoint_6)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i20.GSR = "ENABLED";
    FD1S3AX counter_15_16__i19 (.D(n131), .CK(clk_c), .Q(out_5v_c_7)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i19.GSR = "ENABLED";
    FD1S3AX counter_15_16__i18 (.D(n132), .CK(clk_c), .Q(out_5v_c_6)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i18.GSR = "ENABLED";
    FD1S3AX counter_15_16__i17 (.D(n133), .CK(clk_c), .Q(out_5v_c_5)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i17.GSR = "ENABLED";
    FD1S3AX counter_15_16__i16 (.D(n134), .CK(clk_c), .Q(out_5v_c_4)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i16.GSR = "ENABLED";
    FD1S3AX counter_15_16__i15 (.D(n135), .CK(clk_c), .Q(out_5v_c_3)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i15.GSR = "ENABLED";
    FD1S3AX counter_15_16__i14 (.D(n136), .CK(clk_c), .Q(out_5v_c_2)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i14.GSR = "ENABLED";
    FD1S3AX counter_15_16__i13 (.D(n137), .CK(clk_c), .Q(out_5v_c_1)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i13.GSR = "ENABLED";
    FD1S3AX counter_15_16__i12 (.D(n138), .CK(clk_c), .Q(out_5v_c_0)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i12.GSR = "ENABLED";
    FD1S3AX counter_15_16__i11 (.D(n139_adj_1), .CK(clk_c), .Q(counter[11])) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i11.GSR = "ENABLED";
    FD1S3AX counter_15_16__i10 (.D(n140_adj_2), .CK(clk_c), .Q(n19)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i10.GSR = "ENABLED";
    FD1S3AX counter_15_16__i9 (.D(n141_adj_3), .CK(clk_c), .Q(n20)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i9.GSR = "ENABLED";
    FD1S3AX counter_15_16__i8 (.D(n142_adj_4), .CK(clk_c), .Q(n21)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i8.GSR = "ENABLED";
    FD1S3AX counter_15_16__i7 (.D(n143_adj_5), .CK(clk_c), .Q(n22)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i7.GSR = "ENABLED";
    FD1S3AX counter_15_16__i6 (.D(n144_adj_6), .CK(clk_c), .Q(n23)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i6.GSR = "ENABLED";
    FD1S3AX counter_15_16__i5 (.D(n145_adj_7), .CK(clk_c), .Q(n24)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i5.GSR = "ENABLED";
    FD1S3AX counter_15_16__i4 (.D(n146_adj_8), .CK(clk_c), .Q(n25)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i4.GSR = "ENABLED";
    FD1S3AX counter_15_16__i3 (.D(n147_adj_9), .CK(clk_c), .Q(n26)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i3.GSR = "ENABLED";
    FD1S3AX counter_15_16__i2 (.D(n148_adj_10), .CK(clk_c), .Q(n27)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i2.GSR = "ENABLED";
    FD1S3AX counter_15_16__i1 (.D(n149_adj_11), .CK(clk_c), .Q(n28)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i1.GSR = "ENABLED";
    OB out_5v_pad_13 (.I(testpoint_1), .O(out_5v[13]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_12 (.I(testpoint_2), .O(out_5v[12]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_11 (.I(testpoint_3), .O(out_5v[11]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_10 (.I(testpoint_4), .O(out_5v[10]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_9 (.I(testpoint_5), .O(out_5v[9]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_8 (.I(testpoint_6), .O(out_5v[8]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_7 (.I(out_5v_c_7), .O(out_5v[7]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_6 (.I(out_5v_c_6), .O(out_5v[6]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_5 (.I(out_5v_c_5), .O(out_5v[5]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_4 (.I(out_5v_c_4), .O(out_5v[4]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_3 (.I(out_5v_c_3), .O(out_5v[3]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_2 (.I(out_5v_c_2), .O(out_5v[2]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_1 (.I(out_5v_c_1), .O(out_5v[1]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_5v_pad_0 (.I(out_5v_c_0), .O(out_5v[0]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(4[24:30])
    OB out_3v3_pad_15 (.I(out_3v3_c_15), .O(out_3v3[15]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_14 (.I(out_3v3_c_14), .O(out_3v3[14]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_13 (.I(out_3v3_c_13), .O(out_3v3[13]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_12 (.I(out_3v3_c_12), .O(out_3v3[12]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_11 (.I(out_3v3_c_11), .O(out_3v3[11]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_10 (.I(out_3v3_c_10), .O(out_3v3[10]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_9 (.I(out_3v3_c_9), .O(out_3v3[9]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_8 (.I(out_3v3_c_8), .O(out_3v3[8]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_7 (.I(out_3v3_c_7), .O(out_3v3[7]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_6 (.I(out_3v3_c_6), .O(out_3v3[6]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_5 (.I(out_3v3_c_5), .O(out_3v3[5]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_4 (.I(out_3v3_c_4), .O(out_3v3[4]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_3 (.I(out_3v3_c_3), .O(out_3v3[3]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_2 (.I(out_3v3_c_2), .O(out_3v3[2]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_1 (.I(out_3v3_c_1), .O(out_3v3[1]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB out_3v3_pad_0 (.I(out_3v3_c_0), .O(out_3v3[0]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(5[24:31])
    OB oe_pad (.I(VCC_net), .O(oe));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(6[24:26])
    OBZ spi_miso_pad (.I(GND_net), .T(VCC_net), .O(spi_miso));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(76[12:20])
    FD1S3AX counter_15_16__i0 (.D(n150_adj_12), .CK(clk_c), .Q(n29)) /* synthesis syn_use_carry_chain=1 */ ;   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16__i0.GSR = "ENABLED";
    OB fram_scl_pad (.I(VCC_net), .O(fram_scl));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(13[24:32])
    OBZ fram_sda_pad (.I(GND_net), .T(VCC_net), .O(fram_sda));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(82[12:20])
    CCU2D counter_15_16_add_4_1 (.A0(GND_net), .B0(GND_net), .C0(GND_net), 
          .D0(GND_net), .A1(n29), .B1(GND_net), .C1(GND_net), .D1(GND_net), 
          .COUT(n139), .S1(n150_adj_12));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(31[20:34])
    defparam counter_15_16_add_4_1.INIT0 = 16'hF000;
    defparam counter_15_16_add_4_1.INIT1 = 16'h0555;
    defparam counter_15_16_add_4_1.INJECT1_0 = "NO";
    defparam counter_15_16_add_4_1.INJECT1_1 = "NO";
    OB testpoint_pad_8 (.I(testpoint_c_8), .O(testpoint[8]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_7 (.I(testpoint_c), .O(testpoint[7]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_6 (.I(testpoint_0), .O(testpoint[6]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_5 (.I(testpoint_1), .O(testpoint[5]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_4 (.I(testpoint_2), .O(testpoint[4]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_3 (.I(testpoint_3), .O(testpoint[3]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_2 (.I(testpoint_4), .O(testpoint[2]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_1 (.I(testpoint_5), .O(testpoint[1]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    OB testpoint_pad_0 (.I(testpoint_6), .O(testpoint[0]));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(16[24:33])
    IB clk_pad (.I(clk), .O(clk_c));   // c:/users/frank/documents/embedded firmware/timingcard/top.v(2[24:27])
    
endmodule
//
// Verilog Description of module PUR
// module not written out since it is a black-box. 
//

//
// Verilog Description of module TSALL
// module not written out since it is a black-box. 
//

