-- FinalGROM 99

-- A cartridge for the TI 99 that allows for running ROM and GROM images
-- stored on an SD card
--
-- Copyright (c) 2017 Ralph Benzinger <r@0x01.de>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, see <http://www.gnu.org/licenses/>.


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity logic is

  Port ( -- TI99 bus connections
         ROMS : in std_logic;
         WE : in std_logic;
         GS : in std_logic;
         DBIN : in std_logic;
         GC : in std_logic;
         GR : out std_logic := 'Z';
         BUS_DX : inout std_logic_vector(7 downto 0) := (others => 'Z');
         BUS_AX : in std_logic_vector(12 downto 0);
         -- RAM connections
         RAM_DX : inout std_logic_vector(7 downto 0) := (others => 'Z');
         RAM_AX : out std_logic_vector(19 downto 0);  -- 1 MB
         RAM_OE : out std_logic := '1';
         RAM_WE : out std_logic := '1';
         -- µC control connections
         CCMD : in std_logic_vector(2 downto 0);  -- actions
         CCLK : in std_logic;  -- load clock
         CDX : inout std_logic_vector(7 downto 0) := (others => 'Z');  -- byte transfered
         CX : inout std_logic  -- ROM Write Enable (in) and Select Clock (out)
         );

end logic;


-- Technical Data:
-- 128 banks of 8 KB
-- 5 usable GROMs
-- GROMs occupy banks 123-127, banks 120-122 reserved

-- Actions (CCMD2-CCDMD1-CCMD0):
-- 0--  RUN           run normally, with active selection
-- 100  LOAD SETSIZE  set ROM/GROM start address and mask
-- 101  LOAD SETCONF  activate RAM or/and GRAM mode
-- 110  LOAD RDUMP    dump current image
-- 111  LOAD RLOAD    load new image

-- Set Size
-- 0xxxxxxx  Set binary ROM bank mask
-- 10-xxxxx  Set unary GROM mask
-- 11------  Reset GROM SRAM address

-- Configuration:
-- conf 0  RAM mode
-- conf 1  GRAM mode

architecture Behavioral of logic is

  -- GROM Ready state
  type grstate_t is (GR0, GRZ);
  signal grstate : grstate_t;

  -- current operation
  type action_t is (RUN, LOAD);
  signal action : action_t := LOAD;  -- cart offline
  type subaction_t is (RLOAD, RDUMP, SETSIZE, SETCONF);
  signal subaction : subaction_t := RLOAD;

  -- state change
  signal ocmd2 : std_logic := '1'; -- LOAD
  signal ocmd1 : std_logic := '1'; -- RLOAD
  signal ocmd0 : std_logic := '1';
  signal conf : std_logic_vector(1 downto 0) := (others => '0'); -- all inactive

  -- GROM signals
  signal grom : std_logic_vector(2 downto 0) := (others => '0'); -- 8 GROMs
  signal gromAddr : std_logic_vector(12 downto 0); -- 6K/8K
  signal gromMask : std_logic_vector(4 downto 0) := (others => '0');  -- 5 usable GROMs
  signal gromSel : std_logic := '0';
  signal delay : std_logic;

  -- ROM signals
  signal bank : std_logic_vector(6 downto 0);  -- 128 banks
  signal bankRam : std_logic_vector(6 downto 0);  -- RAM banks
  signal bankMask : std_logic_vector(6 downto 0);  -- mask >00..>7F

  -- image selection signals
  signal selection : std_logic_vector(8 downto 0);  -- clock & byte
  signal romWrite : std_logic := '1';
  
  -- address counters
  signal loadAddr : std_logic_vector(19 downto 0);  -- 1 MB
  signal ramAddr : std_logic_vector(19 downto 0);  -- (4 & 3) & 13

begin

  -- data bus output
  BUS_DX <= -- ROM read
            RAM_DX when action = RUN and ROMS = '0' and DBIN = '1' else
            -- GROM read
            RAM_DX when action = RUN and GS = '0' and DBIN = '1' and
                        BUS_AX(1) = '0' and gromSel = '1' else  -- remove BAX?
            -- no read, don't touch data bus
            (others => 'Z');

  -- SRAM address
  RAM_AX <= -- load address when loading/dumping ROM
            loadAddr when action = LOAD else
            -- current SRAM address for bus access
            ramAddr;

  ramAddr <= -- read from GROM == upper banks
             "1111" & grom & gromAddr when GS = '0' else
             -- read from RAM (if enabled)
             bankRam & BUS_AX when conf(0) = '1' and BUS_AX(12) = '1' else  -- RAM bank
             -- read from ROM
             bank & BUS_AX;

  -- SRAM data
  RAM_DX <= -- write external data when loading ROM
            CDX when action = LOAD and subaction = RLOAD else
            -- write bus data when writing to RAM bank
            BUS_DX when action = RUN and WE = '0' else
            -- otherwise read from SRAM
            (others => 'Z');

  -- read SRAM
  RAM_OE <= -- disable SRAM output when loading image
            '1' when action = LOAD and subaction = RLOAD else
            -- disable SRAM output when updating RAM bank
            '1' when action = RUN and WE = '0' else
            -- else always output
            '0';

  -- write RAM
  RAM_WE <= -- enable SRAM write when loading image
            CX when action = LOAD and subaction = RLOAD and CCLK = '0' else -- CCLK for stability
            -- enable SRAM write when updating RAM bank
            '0' when action = RUN and ROMS = '0' and WE = '0'
                                  and conf(0) = '1' and BUS_AX(12) = '1' else
            -- enable GRAM write for all GROMs
            '0' when action = RUN and GS = '0' and WE = '0'
                                  and conf(1) = '1' and BUS_AX(1) = '0' else
            -- else never write
            '1';

  -- data transfer to uC
  CDX <= selection(7 downto 0) when action = RUN else  -- image selection
         RAM_DX when action = LOAD and subaction = RDUMP else  -- dump image
         (others => 'Z');

  romWrite <= '0' when ROMS = '0' and WE = '0' else '1';
  
  -- clock transfer to uC
  CX <= selection(8) when action = RUN else
        'Z';
  
  -- GROM Ready logic
  with grstate select
    GR <= '0' when GR0,
          'Z' when others;

  -- GROM Ready state machine
  PGRDY: process(GC, GS)
  begin
    if GS = '1' then
      grstate <= GR0;  -- no read -> not ready
    elsif rising_edge(GC) then
      case grstate is
        when GR0 => grstate <= GRZ;  -- wait at least one GROM clock ...
        when GRZ => grstate <= GRZ;  -- ... before being ready
      end case;
    end if;
  end process;

  -- GROM logic
  PGROM: process(GS, action, subaction, CDX)
    variable newGrom : std_logic_vector(2 downto 0);
  begin
    if action = LOAD and subaction = SETSIZE and CDX(7 downto 6) = "10" then
        -- GSET command w/RESET
        grom <= (others => '0');
        gromAddr <= (others => '0');
        gromSel <= '0';
    elsif falling_edge(GS) then  -- observe GROM addrs even when loading/dumping
      if DBIN = '0' and BUS_AX(1) = '1' then
        -- set GROM address
        newGrom := gromAddr(7 downto 5);
        grom <= newGrom;
        gromAddr <= gromAddr(4 downto 0) & BUS_DX;
        case newGrom is  -- check if GROM is loaded
          when "011" => gromSel <= gromMask(0);  -- GROMs 3-7
          when "100" => gromSel <= gromMask(1);
          when "101" => gromSel <= gromMask(2);
          when "110" => gromSel <= gromMask(3);
          when "111" => gromSel <= gromMask(4);
          when others => gromSel <= '0';  -- no GROMs 0-2
        end case;
        delay <= '0';  -- do not increment addr on first read
      elsif BUS_AX(1) = '0' then
        -- auto-increment GROM addr
        gromAddr <= gromAddr + delay;
        delay <= '1';  -- enable GROM auto increment after first read
      end if;
    end if;
  end process;

  -- bank switch logic
  PBANK: process(WE, action, subaction)
  begin
    if action = LOAD and subaction = SETSIZE then
      -- SET command (GSET precedes RSET)
      bank <= (others => '0');
      bankRam <= (others => '0');
    elsif rising_edge(WE) then
      if ROMS = '0' and BUS_AX(0) = '1' then  -- fetch from first byte
      -- could add action = RUN, but who writes to ROM while loading?
        -- select bank
        if conf(0) = '1' and BUS_AX(12 downto 11) = "01" then
          -- switch RAM bank (needs to be enabled)
          bankRam <= BUS_AX(7 downto 1) and bankMask;
        elsif conf(0) = '0' or BUS_AX(12) = '0' then
          -- switch ROM bank
          bank <= BUS_AX(7 downto 1) and bankMask;
        end if;
      end if;
    end if;
  end process;

  -- image selection
  PSELC: process(action, romWrite, BUS_AX)
  begin
    if action = RUN and falling_edge(romWrite) then
      -- select image
      selection <= BUS_AX(12) & BUS_AX(8 downto 1);
    end if;
  end process;

  -- image load and dump address logic
  PLOAD: process(CCLK)
  begin
    if rising_edge(CCLK) then
      if action = LOAD and subaction = SETSIZE then
        -- set where to load
        if CDX(7) = '0' then
          -- load in ROM
          loadAddr <= (others => '0');
          bankMask <= CDX(6 downto 0);  -- valid bank switches
        else
          -- load in GROM
          loadAddr <= "1111011" & (BUS_AX'range => '0');  -- start at GROM 3
          gromMask <= CDX(4 downto 0);  -- valid GROMs
        end if;
      else
        -- load each byte sequentially
        loadAddr <= loadAddr + 1;
      end if;
    end if;
  end process;

  -- mode lock to filter out glitches in control lines
  -- NOTE: it may take up to almost 3 GC cycles  =>  3 * 2,24 us < 7 us
  --       a CPU cycle takes 1/3 us  =>  7 us = 21 CPU cycles
  PMODE: process(GC)
  begin
    if rising_edge(GC) then
      ocmd2 <= CCMD(2);  -- action RUN or LOAD
      ocmd1 <= CCMD(1);  -- subaction
      ocmd0 <= CCMD(0);
      if ocmd2 = CCMD(2) and ocmd1 = CCMD(1) and ocmd0 = CCMD(0) then
        if ocmd2 = '0' then
          action <= RUN;
        else
          action <= LOAD;
        end if;
        if ocmd1 = '0' then
          if ocmd0 = '0' then
            subaction <= SETSIZE;
          else
            subaction <= SETCONF;
            conf <= CDX(1 downto 0);
          end if;
        else
          if ocmd0 = '0' then
            subaction <= RDUMP;
          else
            subaction <= RLOAD;
          end if;
        end if;
      end if;
    end if;
  end process;

end Behavioral;
