-- Citac s volitelnou frekvenci
-- IVH projekt - ukol2
-- autor:

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.Common.all;

-------------------------------------------------
entity FSM is
port(
    clk : in std_logic;
    rst : in std_logic;
    shift_num : in std_logic_vector(5 downto 0);
    state : inout state_type;

    s_saver_reset : out std_logic
);
end entity FSM;

-------------------------------------------------
architecture behavioral of FSM is
begin
	process (CLK)
	begin
		if(rising_edge(CLK)) then
			if RST = '1' then
				state <= init;
			else
				case state is
					when init =>
						state <= right;
					when right =>
						if SHIFT_NUM = 47 then
							state <= left;
						end if;
					when left =>
						if SHIFT_NUM = 47 then
							state <= animation;
						end if;
					when animation => --animation
						if SHIFT_NUM = 15 then
							state <= screen_saver;
						end if;
					when screen_saver => --animation
						if SHIFT_NUM = 47 then
							state <= init;
						end if;
				end case;
			end if;
		end if;
	end process;

	s_saver_reset <= '0' when state = screen_saver else '1';
end behavioral;