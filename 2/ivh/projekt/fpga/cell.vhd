-- Bunka - ovlada jednu ledku
-- IVH projekt
-- autor: Miroslav Harag (xharag02)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.common.all;

entity cell is
    Generic (
        x : integer := 0;
        y : integer := 0);
    Port (
        clk         : in  std_logic;
        reset       : in  std_logic;
        state       : out std_logic;
        init_state  : in  std_logic;
        neight_left : in  std_logic;
        neight_right    : in  std_logic;
        fsm_state       : in state_type;
        animation_num   : in std_logic_vector(5 downto 0)
    );
end cell;

architecture Behavioral of cell is
begin
    process (CLK)
    begin
        if (rising_edge(CLK)) then
            if reset = '1' then
                state <= '0';
            else
                case FSM_STATE is
                    when init =>
                        state <= init_state;
                    when left => 
                        state <= neight_left;
                    when right => 
                        state <= neight_right;
                    when others => 
                        if (animation_num = x and (y mod 2 = 1)) or (animation_num = 16-x and (y mod 2 = 0)) then
                            state <= '1';
                        end if;
                end case;  
            end if;
        end if;
    end process;

end Behavioral;