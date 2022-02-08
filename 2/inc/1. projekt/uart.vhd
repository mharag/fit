-- uart.vhd: UART controller - receiving part
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

-------------------------------------------------
entity UART_RX is
port(	
   CLK: 	    in std_logic;
	RST: 	    in std_logic;
	DIN: 	    in std_logic;
	DOUT: 	    out std_logic_vector(7 downto 0);
	DOUT_VLD: 	out std_logic
);
end UART_RX;  

-------------------------------------------------
architecture behavioral of UART_RX is
	signal baud_tick : std_logic;
	signal read_enable : std_logic;
	signal bit_num : std_logic_vector (2 downto 0);
	signal baud_tick_RST : std_logic;
	signal bit_num_RST : std_logic;
	signal output_RST : std_logic;
	
	signal CNT16 : std_logic_vector (3 downto 0);
begin
	FSM: entity work.UART_FSM(behavioral)
		port map (
         CLK => CLK,
			RST => RST,
			DIN => DIN,
			baud_tick => baud_tick,
			bit_num => bit_num,
			
			baud_tick_RST => baud_tick_RST,
			bit_num_RST => bit_num_RST,
			output_RST => output_RST,
			output_VLD => DOUT_VLD
	);
	
	CLK16: process (CLK, baud_tick_RST)
	begin
		if ( baud_tick_RST = '1' ) then
			CNT16 <= (others => '0');
		elsif ( rising_edge(CLK) ) then
		 	 CNT16 <= CNT16 + 1;
		end if;
   end process;
	baud_tick <= '1' when CNT16 = "1111" else '0';
	read_enable <= '1' when CNT16 = "0111" else '0';
	 
	BIT_CNT: process (baud_tick, bit_num_RST)
	begin
		if ( bit_num_RST = '1' ) then
			bit_num <= (others => '0');
		elsif ( rising_edge(baud_tick) ) then
			bit_num <= bit_num + 1;
		end if;
	end process;
	
	read_bit: process (read_enable, output_RST)
	begin
		if ( output_RST = '1' ) then
			DOUT <= (others => '0');
		elsif ( read_enable = '1' ) then
			DOUT(conv_integer(bit_num)) <= DIN;
		end if;
	end process;

end behavioral;
