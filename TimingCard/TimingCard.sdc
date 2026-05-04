# Clocks used by LSE during synthesis. The LPF still owns placement and I/O.
create_clock -name clk -period 100.000 [get_ports {clk}]
create_clock -name spi_sck -period 250.000 [get_ports {spi_sck}]
