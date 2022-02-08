-- Screen saver
-- IVH projekt
-- autor: Miroslav Harag (xharag02)

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.Common.all;

architecture main of tlv_gp_ifc is
    signal a : std_logic_vector(3 downto 0) := (others => '0');
    signal r : std_logic_vector(7 downto 0);
    signal data_init : std_logic_vector(127 downto 0) := -- obraz na vykreslovanie
        "0000000000000000" &
        "0100001001000010" &
        "0110011001000010" &
        "0101101001000010" &
        "0100001001111110" &
        "0100001001000010" &
        "0100001001000010" &
        "0000000000000000";


    signal data          : std_logic_vector(127 downto 0);
    signal data_cell     : std_logic_vector(127 downto 0); -- vystup screen saver
    signal data_s_saver  : std_logic_vector(127 downto 0); -- vystup novin

    signal display_clk   : std_logic;
    signal fsm_clk       : std_logic; -- automat
    signal animation_clk : std_logic; -- screen_saver

    signal shift_num     : std_logic_vector(5 downto 0);

    signal fsm_state     : state_type;
    signal s_saver_reset : std_logic;


begin
    -- hodiny
    display_clk_gen : entity work.counter
        generic map (clk_freq => 25000000, out_freq => 1024)
        port map (clk,reset,display_clk);
    fsm_clk_gen : entity work.counter
        generic map (clk_freq => 25000000, out_freq => 3)
        port map (clk,reset,fsm_clk);
    animation_clk_gen : entity work.counter
        generic map (clk_freq => 25000000, out_freq => 13)
        port map (clk,reset,animation_clk);

    -- counter - pocita kolko krat sme sa uz posunuli dolava/doprava
    process ( fsm_clk )
    begin
        if rising_edge( fsm_clk ) then
            if( shift_num = 47 or fsm_state = init) then
                shift_num <= (others => '0');
            else
                shift_num <= shift_num + 1;
            end if;
        end if;
    end process;

    -- riadiaci automat
    fsm : entity work.fsm
        port map (
            clk => fsm_clk,
            rst => reset,
            shift_num => shift_num,
            state => fsm_state,
            s_saver_reset => s_saver_reset
        );

    -- podla stavu automatu sa prepina medzi novinami a screen saver
    data <= data_s_saver when fsm_state = screen_saver else data_cell;
    -- vykreslovanie na display
    dis : entity work.display port map (
        clk => clk,
        smclk => smclk,
        reset => reset,
        data => data,
        a => a,
        r => r
    );

    screen_saver : entity work.screen_saver port map (
        clk     => animation_clk,
        reset   => s_saver_reset,
        en      => '1',
        data    => data_s_saver
    );

    rows: for x in 15 downto 0 generate
        cols: for y in 7 downto 0 generate
            cell : entity work.cell
            generic map (x,y)
            port map (  
                clk     => fsm_clk,
                reset   => reset,
                state   => data_cell(getid(x,y,8,16)),
                init_state      => data_init(getid(x,y,8,16)),
                neight_left     => data_cell(getid(x-1,y,8,16)),
                neight_right    => data_cell(getid(x+1,y,8,16)),
                fsm_state       => fsm_state,
                animation_num   => shift_num
            );
        end generate;
    end generate;






    -- mapovani vystupu
    -- nemenit
    X(6) <= A(3);
    X(8) <= A(1);
    X(10) <= A(0);
    X(7) <= '0'; -- en_n
    X(9) <= A(2);

    X(16) <= R(1);
    X(18) <= R(0);
    X(20) <= R(7);
    X(22) <= R(2);

    X(17) <= R(4);
    X(19) <= R(3);
    X(21) <= R(6);
    X(23) <= R(5);
end main;
