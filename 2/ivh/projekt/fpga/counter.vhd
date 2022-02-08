-- Citac s volitelnou frekvenci
-- IVH projekt - ukol2
-- autor:

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
-- v pripade nutnosti muzete nacist dalsi knihovny
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.numeric_std.ALL;

entity counter is
	 Generic (
			CLK_FREQ : positive := 100000;
			OUT_FREQ : positive := 10000);			
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           EN : out  STD_LOGIC);
end counter;

architecture Behavioral of counter is

	function log2(A: integer) return integer is
		variable bits : integer := 0;
		variable b : integer := 1;
	begin
		while (b <= a) loop
			b := b * 2;
			bits := bits + 1;
		end loop;
		return bits;
	end function;

	constant divider : integer := CLK_FREQ/OUT_FREQ;	
	signal counter : STD_LOGIC_VECTOR (log2(divider) downto 0) := (others => '0');

begin 
	process (CLK)
	begin
		if (rising_edge(CLK)) then
			if ((RESET = '1') or (COUNTER = divider-1)) then
				COUNTER <= (others => '0');
			else
				COUNTER <= COUNTER + 1; 
			end if;
		end if;
	end process;

	EN <= '1' when (COUNTER = divider-1) else '0';

end Behavioral;