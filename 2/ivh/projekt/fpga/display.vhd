-- Blikani ledkami
-- uvodni priklad pro IVH - Vojtech Mrazek
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- DO entity nezasahujte, bude pouzita k simulaci!
entity display is
	port (
		data : in std_logic_vector(127 downto 0); -- data k zobrazeni, format je na popsany dole
		reset : in std_logic;
		clk : in std_logic; -- hodiny 25 MHz
		smclk : in std_logic; -- hodiny 7.3728MHz
		A : out std_logic_vector(3 downto 0);
		R : out std_logic_vector(7 downto 0)
	);
end entity display;

architecture behv of display is
   constant TMR : natural := 20; -- 18 je normalni

   signal cnt : std_logic_vector(23 downto 0) := (others => '0');
   signal A_temp : std_logic_vector(3 downto 0) := (others => '0');
   signal R_temp : std_logic_vector(7 downto 0);
   signal freq : std_logic;  
   signal freqsl : std_logic; 

	-- definujte si libovolne signaly
begin

-- Vystupni logika. Vas ukol: vse smazat a naimplementovat zobrazeni matice z vektoru data
-- mapovani by melo odpovidat vasi funkci GETID

-- cilem je to, aby byl text citelny!

	counter : entity work.counter 
		generic map (CLK_FREQ => 25000000, OUT_FREQ => 1024)
		port map (clk,reset,freq);

	posun_riadku : process( freq, RESET )
	begin
		A_temp <= A_temp;
		if (RESET = '1') then
			A_temp <= (others => '0');
		else
			if( rising_edge(freq) )then
				A_temp <= A_temp + 1;
			end if ;
		end if;
	end process ;

	A <= A_temp;


	map_led: for i in 7 downto 0 generate
		-- stlpce na fitkite sa vypisuju zrkadlovo 
		-- (8-i)*16-1 znaci koniec riadku
		R(i) <= data(((8-i)*16-1)-conv_integer(A_temp));
	end generate;
end behv;
