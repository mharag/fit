BASE = ../../../
FCMAKE = C:/Program Files/FITkit/bin/fcmake.exe
FKFLASH = C:/Program Files/FITkit/bin/fkflash.exe
FKTERM = C:/Program Files/FITkit/bin/fkterm.exe
FKTEST = C:/Program Files/FITkit/bin/fktest.exe
PROJECT = project.xml
OUTPUTPREFIX = proj3
FPGACHIP = xc3s50
FPGASPEEDGRADE = 4
FPGAPACKAGE = pq208

all: dependencycheck build/proj3_f1xx.hex build/proj3_f2xx.hex build/proj3.bin

#MCU part
#=====================================================================
HEXFILE_F1XX = build/proj3_f1xx.hex
HEXFILE_F2XX = build/proj3_f2xx.hex

build/mcu/main_f1xx.o: mcu/main.c
	$(comp_tpl_f1xx)

build/mcu/main_f2xx.o: mcu/main.c
	$(comp_tpl_f2xx)

OBJFILES_F1XX = build/mcu/main_f1xx.o
OBJFILES_F2XX = build/mcu/main_f2xx.o

#FPGA part
#=====================================================================
BINFILE = build/proj3.bin
HDLFILES  = ../../../fpga/units/clkgen/clkgen_config.vhd
HDLFILES += ../../../fpga/units/clkgen/clkgen.vhd
HDLFILES += ../../../fpga/units/math/math_pack.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_entity.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_autoincr.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_reg.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_ctrl.vhd
HDLFILES += ../../../fpga/chips/fpga_xc3s50.vhd
HDLFILES += ../../../fpga/chips/architecture_gp/arch_gp_ifc.vhd
HDLFILES += ../../../fpga/chips/architecture_gp/tlv_gp_ifc.vhd
HDLFILES += fpga/top.vhd
HDLFILES += fpga/display.vhd
HDLFILES += fpga/counter.vhd
HDLFILES += fpga/cell.vhd
HDLFILES += fpga/fsm.vhd
HDLFILES += fpga/screen_saver.vhd
HDLFILES += fpga/common.vhd

build/proj3.bin: build/fpga/proj3.par.ncd build/fpga/proj3.pcf

PKGS_LIST = ../../../fpga/units/clkgen/package.xml
PKGS_LIST += ../../../fpga/units/math/package.xml
PKGS_LIST += ../../../fpga/ctrls/spi/package.xml
PKGS_LIST += ../../../fpga/chips/architecture_gp/package.xml

include $(BASE)/base/Makefile.inc
