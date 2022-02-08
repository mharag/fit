-- Screen saver
-- IVH projekt
-- autor: Miroslav Harag (xharag02)

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;
use WORK.COMMON.all;

entity screen_saver is
    Port ( 
        clk     : in  STD_LOGIC;
        reset   : in  STD_LOGIC;
        en      : in std_logic;
        data    : out std_logic_vector(127 downto 0)
    );
end screen_saver;

architecture Behavioral of screen_saver is
begin
    process (CLK)
        variable index : std_logic_vector(6 downto 0); -- horne 4 bity su y, dolne 3 x
        variable history : std_logic_vector(34 downto 0); -- poslednych 5 pozicii, tvori "hadika"

        variable x_dir : std_logic; -- urcuje smer hadika v x
        variable y_dir : std_logic; -- urcuje smer hadika v y

        variable clear : std_logic := '0'; -- proces pracuje v 2 fazach - najskor zapise novy bod a v dalsom takte vymaze posledny
    begin
        if (rising_edge(CLK)) then
            if( reset = '1') then
                index := "0100111"; -- nahodna hodnota - zaciatok
                history := (others => '0');
                y_dir := '0';
                x_dir := '0';
                data <= (others => '1');
                clear := '0'; -- najskor sa musi nieco zapisat
            elsif clear = '0' then -- zapisovanie noveho bodu
                if index(3 downto 0) = 0 or index(3 downto 0) = 15 then -- ak narazi na koniec zmeni smer
                    x_dir := not x_dir;
                end if;

                if index(6 downto 4) = 0 or index(6 downto 4) = 7 then
                    y_dir := not y_dir;
                end if;

                if x_dir = '1' then -- posun v x smere
                    index := index+1;
                else
                    index := index-1;
                end if;
                if y_dir = '1' then -- posun v y smere
                    index := index+16;
                else
                    index := index-16;
                end if;

                history(34 downto 28) :=  index; -- zapis do historie
                data(conv_integer(index)) <= '0';
                clear := '1';
            else -- odstranenie posledneho body z hadika
                data(conv_integer(history(6 downto 0))) <= '1';
                history := "0000000"&history (34 downto 7); -- posunutie historie 
                clear := '0';
            end if;
        end if;
    end process;
end Behavioral;