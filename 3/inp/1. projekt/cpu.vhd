-- cpu.vhd: Simple 8-bit CPU (BrainLove interpreter)
-- Copyright (C) 2021 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Miroslav Harag
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- ram[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_WREN  : out std_logic;                    -- cteni z pameti (DATA_WREN='0') / zapis do pameti (DATA_WREN='1')
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA obsahuje stisknuty znak klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna pokud IN_VLD='1'
   IN_REQ    : out std_logic;                     -- pozadavek na vstup dat z klavesnice
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- pokud OUT_BUSY='1', LCD je zaneprazdnen, nelze zapisovat,  OUT_WREN musi byt '0'
   OUT_WREN : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

 -- zde dopiste potrebne deklarace signalu

	type FSMstate is (
		FETCH_SET, 
		FETCH_WAIT, 
		DECODE, 
		EXECUTE,
		
		DATA_PTR_INC,		
		DATA_PTR_DEC,
		
		VALUE_INC_LOAD_SET,
		VALUE_INC_LOAD_WAIT,
		VALUE_INC_SAVE,

		VALUE_DEC_LOAD_SET,
		VALUE_DEC_LOAD_WAIT,
		VALUE_DEC_SAVE,	
		
		OUTPUT_WAIT,
		OUTPUT_LOAD,
		OUTPUT_SEND,
		
		INPUT_SET,
		INPUT_WAIT,
		INPUT_SAVE,
		
		LOOP_START,
		LOOP_START_LOAD,
		LOOP_START_IS_ZERO,
		LOOP_SKIP,
		LOOP_SKIP_LOAD_CHAR,
		LOOP_SKIP_LOAD_CHAR_WAIT,
		LOOP_SKIP_CHAR,
		LOOP_SKIP_CONTINUE,
		
		LOOP_END,
		LOOP_END_LOAD,
		LOOP_END_IS_ZERO,
		LOOP_RETURN,
		LOOP_RETURN_WAIT,
		LOOP_RETURN_LOAD_CHAR,
		LOOP_RETURN_LOAD_CHAR_WAIT,
		LOOP_RETURN_CHAR,
		LOOP_RETURN_CHAR_PC,
		LOOP_RETURN_CONTINUE,
		
		BREAK,
		BREAK_LOAD,
		BREAK_LOAD_WAIT,
		BREAK_SKIP,
		BREAK_WAIT,
		
		HALT 
	);
	signal pstate : FSMstate;	
	signal nstate : FSMstate;
	
	signal PC_inc : std_logic;
	signal PC_dec : std_logic;
	signal PC_cnt : std_logic_vector(11 downto 0);
	
	signal PTR_inc : std_logic;
	signal PTR_dec : std_logic;
	signal PTR_cnt : std_logic_vector(9 downto 0);
	
	signal cnt_inc : std_logic;
	signal cnt_dec : std_logic;
	signal cnt : std_logic_vector(11 downto 0);
	signal cnt_zero : std_logic;
	
	signal VALUE_zero : std_logic;
	
	signal sel : std_logic_vector(1 downto 0);

begin
 -- zde dopiste vlastni VHDL kod

 -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
 --   - nelze z vice procesu ovladat stejny signal,
 --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
 --      - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
 --      - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly.
	
	--Present State register
	pstatereg: process(RESET, CLK, EN)
	begin
		if (RESET='1') or (EN = '0') then
			pstate <= FETCH_SET;
		elsif (CLK'event) and (CLK='1') then
			pstate <= nstate;
		end if;
	end process;
	
	--Next State logic
	nstate_logic: process(pstate, OUT_BUSY, IN_VLD, VALUE_zero, cnt_zero, CODE_DATA)
	begin
		nstate <= FETCH_SET;
		case pstate is
			when FETCH_SET =>
				nstate <= FETCH_WAIT;
			when FETCH_WAIT =>
				nstate <= DECODE;
			when DECODE =>
				case CODE_DATA is
					when X"3E" => -- ">"
						nstate <= DATA_PTR_INC;
					when X"3C" => -- "<"
						nstate <= DATA_PTR_DEC;
					when X"2B" => -- "+"
						nstate <= VALUE_INC_LOAD_SET;
					when X"2D" => -- "-"
						nstate <= VALUE_DEC_LOAD_SET;
					when X"2E" => -- "."
						nstate <= OUTPUT_WAIT;
					when X"2C" => -- ","
						nstate <= INPUT_SET;
					when X"5B" => -- "["
						nstate <= LOOP_START;
					when X"5D" => -- "]"
						nstate <= LOOP_END;
					when X"7E" => -- "~"
						nstate <= BREAK;
					when X"00" => -- "KONIEC PROGRAMU"
						nstate <= HALT;
					when others =>
						nstate <= EXECUTE;
				end case;
			when DATA_PTR_INC =>
				nstate <= FETCH_SET;
			when DATA_PTR_DEC =>
				nstate <= FETCH_SET;
			
			when VALUE_INC_LOAD_SET =>
				nstate <= VALUE_INC_LOAD_WAIT;
			when VALUE_INC_LOAD_WAIT =>
				nstate <= VALUE_INC_SAVE;
			when VALUE_INC_SAVE =>
				nstate <= FETCH_SET;
			
			when VALUE_DEC_LOAD_SET =>
				nstate <= VALUE_DEC_LOAD_WAIT;
			when VALUE_DEC_LOAD_WAIT =>
				nstate <= VALUE_DEC_SAVE;
			when VALUE_DEC_SAVE =>
				nstate <= FETCH_SET;
				
			when OUTPUT_WAIT =>
				if OUT_BUSY = '1' then
					nstate <= OUTPUT_WAIT;
				else 
					nstate <= OUTPUT_LOAD;
				end if;
			when OUTPUT_LOAD =>
				nstate <= OUTPUT_SEND;
			when OUTPUT_SEND =>
				nstate <= FETCH_SET;
				
			when INPUT_SET =>
				nstate <= INPUT_WAIT;
			when INPUT_WAIT =>
				if IN_VLD = '0' then
					nstate <= INPUT_WAIT;
				else
					nstate <= INPUT_SAVE;
				end if;
			when INPUT_SAVE =>
				nstate <= FETCH_SET;
				
			-- Zaciatok cyklu
			when LOOP_START =>
				nstate <= LOOP_START_LOAD;
			when LOOP_START_LOAD =>
				nstate <= LOOP_START_IS_ZERO;
			when LOOP_START_IS_ZERO =>
				if VALUE_zero = '1' then
					nstate <= LOOP_SKIP;
				else
					nstate <= FETCH_SET;
				end if;
			when LOOP_SKIP =>
				nstate <= LOOP_SKIP_LOAD_CHAR;
			when LOOP_SKIP_LOAD_CHAR =>
				nstate <= LOOP_SKIP_LOAD_CHAR_WAIT;
			when LOOP_SKIP_LOAD_CHAR_WAIT =>
				nstate <= LOOP_SKIP_CHAR;
			when LOOP_SKIP_CHAR =>
				nstate <= LOOP_SKIP_CONTINUE;
			when LOOP_SKIP_CONTINUE =>
				if cnt_zero = '1' then
					nstate <= FETCH_SET;
				else 
					nstate <= LOOP_SKIP_LOAD_CHAR;
				end if;
				
			-- Koniec cyklu
			when LOOP_END =>
				nstate <= LOOP_END_LOAD;
			when LOOP_END_LOAD =>
				nstate <= LOOP_END_IS_ZERO;
			when LOOP_END_IS_ZERO =>
				if VALUE_zero = '1' then
					nstate <= EXECUTE;
				else
					nstate <= LOOP_RETURN;
				end if;
			when LOOP_RETURN =>
				nstate <= LOOP_RETURN_WAIT;
			when LOOP_RETURN_WAIT =>
				nstate <= LOOP_RETURN_LOAD_CHAR;
			when LOOP_RETURN_LOAD_CHAR =>
				nstate <= LOOP_RETURN_LOAD_CHAR_WAIT;
			when LOOP_RETURN_LOAD_CHAR_WAIT =>
				nstate <= LOOP_RETURN_CHAR;
			when LOOP_RETURN_CHAR =>
				nstate <= LOOP_RETURN_CHAR_PC;
			when LOOP_RETURN_CHAR_PC =>
				nstate <= LOOP_RETURN_CONTINUE;
			when LOOP_RETURN_CONTINUE =>
				if cnt_zero = '1' then
					nstate <= FETCH_SET;
				else 
					nstate <= LOOP_RETURN_LOAD_CHAR;
				end if;
				
			when BREAK =>
				nstate <= BREAK_LOAD;
			when BREAK_LOAD =>
				nstate <= BREAK_LOAD_WAIT;
			when BREAK_LOAD_WAIT =>
				if cnt_zero = '1' then
					nstate <= FETCH_SET;
				else 
					nstate <= BREAK_SKIP;
				end if;
			when BREAK_SKIP =>
				nstate <=BREAK_WAIT;
			when BREAK_WAIT =>
				nstate <= BREAK_LOAD;
			when HALT =>
				nstate <= HALT;
				
			when others => null;
		end case;
	end process;
	--Output logic
	output_logic: process(pstate, CODE_DATA, cnt_zero)
	begin
		CODE_EN <= '0';
		
		DATA_WREN <= '0';
		DATA_EN <= '0';
		sel <= (others => '0');
		
		OUT_WREN <= '0';
		
		IN_REQ <= '0';
		
		OUT_DATA <= (others => '0');
		
		PC_inc <= '0';
		PC_dec <= '0';
		PTR_inc <= '0';
		PTR_dec <= '0';
		cnt_inc <= '0';
		cnt_dec <= '0';
		case pstate is
			when FETCH_SET =>
				CODE_EN <= '1'; 
			when EXECUTE =>
				PC_inc <= '1';
				
			when DATA_PTR_INC =>
				PTR_inc <= '1';
				PC_inc <= '1';
			when DATA_PTR_DEC => 
				PTR_dec <= '1';
				PC_inc <= '1';
				
			when VALUE_INC_LOAD_SET | VALUE_DEC_LOAD_SET =>
				DATA_EN <= '1';
			when VALUE_INC_SAVE =>
				DATA_WREN <= '1';
				DATA_EN <= '1';
				sel <= "10";
				PC_inc <= '1';
			when VALUE_DEC_SAVE =>
				DATA_WREN <= '1';
				DATA_EN <= '1';
				sel <= "11";
				PC_inc <= '1';
				
			when OUTPUT_WAIT =>
				DATA_EN <= '1';
			when OUTPUT_SEND =>
				OUT_DATA <= DATA_RDATA;
				OUT_WREN <= '1';
				PC_inc <= '1';
				
			when INPUT_SET =>
				IN_REQ <= '1';
			when INPUT_SAVE =>
				DATA_WREN <= '1';
				DATA_EN <= '1';
				sel <= "01";
				PC_inc <= '1';
				
			when LOOP_START =>
				DATA_EN <= '1';
				PC_inc <= '1';
			when LOOP_SKIP =>
				cnt_inc <= '1';
			when LOOP_SKIP_LOAD_CHAR =>
				CODE_EN <= '1';
			when LOOP_SKIP_CHAR =>
				if CODE_DATA = X"5B" then
					cnt_inc <= '1';
				end if;
				if CODE_DATA = X"5D" then
					cnt_dec <= '1';
				end if;
				PC_inc <= '1';
			
			when LOOP_END =>
				DATA_EN <= '1';
			when LOOP_RETURN =>
				cnt_inc <= '1';
				PC_dec <= '1'; 
			when LOOP_RETURN_LOAD_CHAR =>
				CODE_EN <= '1';
			when LOOP_RETURN_CHAR =>
				if CODE_DATA = X"5D" then
					cnt_inc <= '1';
				end if;
				if CODE_DATA = X"5B" then
					cnt_dec <= '1';
				end if;
			when LOOP_RETURN_CHAR_PC =>
				if cnt_zero = '1' and CODE_DATA = X"5B" then
					PC_inc <= '1';
				else
					PC_dec <= '1';
				end if;
				
			when BREAK =>
				PC_inc <= '1';
				cnt_inc <= '1';
			when BREAK_LOAD =>
				CODE_EN <= '1';
			when BREAK_SKIP =>
				if CODE_DATA = X"5B" then 
					cnt_inc <= '1';
				end if;
				if CODE_DATA = X"5D" then
					cnt_dec <= '1';
				end if;
				PC_inc <= '1';
				
			when others => null;
		end case;
	end process;
 
	PC: process(CLK, RESET)
	begin
		if RESET = '1' then
			PC_cnt <= (others => '0');
		elsif CLK'event and CLK = '1' then
			if PC_inc = '1' then
				PC_cnt <= PC_cnt + 1;
			end if;
			if PC_dec = '1' then
				PC_cnt <= PC_cnt - 1;
			end if;
		end if;
	end process;
	CODE_ADDR <= PC_cnt;
	
	PTR: process(CLK, RESET)
	begin
		if RESET = '1' then
			PTR_cnt <= (others => '0');
		elsif CLK'event and CLK = '1' then
			if PTR_inc = '1' then
				PTR_cnt <= PTR_cnt + 1;
			end if;
			if PTR_dec = '1' then
				PTR_cnt <= PTR_cnt - 1;
			end if;
		end if;
	end process;
	DATA_ADDR <= PTR_cnt;
	
	CNT_process: process(CLK, RESET)
	begin
		if RESET = '1' then
			cnt <= (others => '0');
		elsif CLK'event and CLK = '1' then
			if cnt_inc = '1' then
				cnt <= cnt + 1;
			end if;
			if cnt_dec = '1' then
				cnt <= cnt - 1;
			end if;
		end if;
	end process;
	
	process (DATA_RDATA, IN_DATA, sel) is
	begin
		if sel = "00" then
			DATA_WDATA <= (others => '0');
		elsif sel = "01" then
			DATA_WDATA <= IN_DATA;
		elsif sel = "10" then
			DATA_WDATA <= DATA_RDATA + 1;
		else
			DATA_WDATA <= DATA_RDATA - 1;
		end if;
	end process;
  
  
	process (cnt) is
	begin
		if cnt = X"000" then
			CNT_zero <= '1';
		else
			CNT_zero <= '0';
		end if;
	end process;
  
	process (DATA_RDATA) is
	begin
		if DATA_RDATA = X"000" then
			VALUE_zero <= '1';
		else
			VALUE_zero <= '0';
	end if;
	end process;

 
end behavioral;
