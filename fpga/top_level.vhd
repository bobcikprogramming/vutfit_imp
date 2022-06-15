library IEEE;
use ieee.std_logic_1164.ALL;
use ieee.std_logic_ARITH.ALL;
use ieee.std_logic_UNSIGNED.ALL;



architecture main of tlv_bare_ifc is
   -----------------------------------------------------------------------------
   -- Signaly

   -- Keyboard 4x4
   signal key_data_in : std_logic_vector (15 downto 0);

   -----------------------------------------------------------------------------
   -- POUZITE KOMPONENTY
   component SPI_adc
      generic (
         ADDR_WIDTH : integer;
         DATA_WIDTH : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR  : integer
      );
      port (
         CLK      : in  std_logic;

         CS       : in  std_logic;
         DO       : in  std_logic;
         DO_VLD   : in  std_logic;
         DI       : out std_logic;
         DI_REQ   : in  std_logic;

         ADDR     : out  std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out  std_logic_vector (DATA_WIDTH-1 downto 0);
         DATA_IN  : in   std_logic_vector (DATA_WIDTH-1 downto 0);

         WRITE_EN : out  std_logic;
         READ_EN  : out  std_logic
      );
   end component;

   -- Keyboard 4x4
   component keyboard_controller
      port(
         CLK      : in std_logic;
         RST      : in std_logic;

         DATA_OUT : out std_logic_vector(15 downto 0);
         DATA_VLD : out std_logic;

         KB_KIN   : out std_logic_vector(3 downto 0);
         KB_KOUT  : in  std_logic_vector(3 downto 0)
      );
   end component;


   component lcd_raw_controller
      port (
         RST      : in std_logic;
         CLK      : in std_logic;

         DATA_IN  : in std_logic_vector (15 downto 0);
         WRITE_EN : in std_logic;

         DISPLAY_RS   : out   std_logic;
         DISPLAY_DATA : inout std_logic_vector(7 downto 0);
         DISPLAY_RW   : out   std_logic;
         DISPLAY_EN   : out   std_logic
      );
   end component;

   -- displej
   signal dis_data_out : std_logic_vector(15 downto 0);
   signal dis_write_en : std_logic;

begin

   LEDF  <= '0';

      -- Adresovy dekoder
   SPI_adc_keybrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#0002# -- adresovy prostor od 0x0002
      )
      port map(
         CLK   =>  CLK,
   
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
   
         ADDR     => open,
         DATA_OUT => open,
         DATA_IN  => key_data_in,
         WRITE_EN => open,
         READ_EN  => open
      );

   -- SPI dekoder pro displej
   spidecd: SPI_adc
         generic map (
            ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
            DATA_WIDTH => 16,    -- sirka dat 16 bitu
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#00# -- adresovy prostor od 0x00-0x01
         )
         port map (
            CLK      => CLK,
            CS       => SPI_CS,

            DO       => SPI_DO,
            DO_VLD   => SPI_DO_VLD,
            DI       => SPI_DI,
            DI_REQ   => SPI_DI_REQ,

            ADDR     => open,
            DATA_OUT => dis_data_out,
            DATA_IN  => "0000000000000000",
            WRITE_EN => dis_write_en,
            READ_EN  => open
         );

   -- Radic klavesnice
   keybrd: keyboard_controller
      port map(
         CLK      => CLK,
         RST      => RESET,       -- reset
         
         DATA_OUT => key_data_in,         
         DATA_VLD => open,

         -- Keyboart
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

   -- radic LCD displeje
   lcdctrl: lcd_raw_controller
      port map (
         RST    =>  RESET,
         CLK    =>  CLK,

         -- ridici signaly
         DATA_IN  => dis_data_out,
         WRITE_EN => dis_write_en,

         --- signaly displeje
         DISPLAY_RS   => LRS,
         DISPLAY_DATA => LD,
         DISPLAY_RW   => LRW,
         DISPLAY_EN   => LE
      );

end main;


