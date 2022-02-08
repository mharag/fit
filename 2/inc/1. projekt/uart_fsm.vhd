-- uart_fsm.vhd: UART controller - finite state machine
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;

-------------------------------------------------
entity UART_FSM is
port(
   CLK : in std_logic;
   RST : in std_logic;
	DIN : in std_logic;
	baud_tick : in std_logic;
	bit_num : in std_logic_vector (2 downto 0);
	
	baud_tick_RST : out std_logic;
	bit_num_RST : out std_logic;
	output_RST : out std_logic;
	output_VLD : out std_logic
   );
end entity UART_FSM;

-------------------------------------------------
architecture behavioral of UART_FSM is
	TYPE STATE_type IS (idle,start_bit,read_byte,end_bit); 
	SIGNAL state : STATE_Type := idle;
begin
	fsm : process ( CLK )
	begin
		if ( RST = '1' ) then 
			state <= idle;
		elsif rising_edge ( CLK ) then
			case state is 
				when idle =>
					if ( DIN = '0' ) then
						state <= start_bit;
					end if;
				when start_bit =>
					if ( baud_tick = '1' ) then
						state <= read_byte;
					end if;
				when read_byte =>
					if ( bit_num = "000" and baud_tick = '1' ) then
						state <= end_bit;
					end if;
				when end_bit =>
					if ( baud_tick = '1' ) then
						state <= idle;
					end if;
			end case;
		end if;
	end process;
	
	-- Moore
	baud_tick_RST <= '1' when state = idle else '0';
	bit_num_RST <= '0' when state = read_byte else '1';
	output_RST <= '1' when state = idle else '0';
	
	-- Mealy
	output_VLD <= '1' when state = read_byte and bit_num = "000" and baud_tick = '1' else '0';

end behavioral;
