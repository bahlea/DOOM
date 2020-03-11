################################################################
#
# $Id:$
#
# $Log:$
#
CC=  riscv64-unknown-elf-gcc  # gcc or g++

RISCV_COMMON = $(PARANUT_HOME)/sw/riscv_common
SYSTEMS_DIR = $(PARANUT_HOME)/systems

CFLAGS=-O2 -g -march=rv32im -mabi=ilp32 -DNORMALUNIX # -DUSEASM 
LDFLAGS= -march=rv32im -mabi=ilp32 -static -Ivears -nostartfiles $(RISCV_COMMON)/startup.S $(RISCV_COMMON)/syscalls.c -T $(RISCV_COMMON)/paranut.ld #-specs=de2.specs
LIBS=-lm

# subdirectory for objects
O=paranut

# not too sophisticated dependency
OBJS=				\
		$(O)/doomdef.o		\
		$(O)/doomstat.o		\
		$(O)/dstrings.o		\
		$(O)/i_sound.o \
		$(O)/i_video.o \
		$(O)/i_net.o \
		$(O)/i_system.o		\
		$(O)/tables.o			\
		$(O)/f_finale.o		\
		$(O)/f_wipe.o 		\
		$(O)/d_main.o			\
		$(O)/d_net.o			\
		$(O)/d_items.o		\
		$(O)/g_game.o			\
		$(O)/m_menu.o			\
		$(O)/m_misc.o			\
		$(O)/m_argv.o  		\
		$(O)/m_bbox.o			\
		$(O)/m_fixed.o		\
		$(O)/m_swap.o			\
		$(O)/m_cheat.o		\
		$(O)/m_random.o		\
		$(O)/am_map.o			\
		$(O)/p_ceilng.o		\
		$(O)/p_doors.o		\
		$(O)/p_enemy.o		\
		$(O)/p_floor.o		\
		$(O)/p_inter.o		\
		$(O)/p_lights.o		\
		$(O)/p_map.o			\
		$(O)/p_maputl.o		\
		$(O)/p_plats.o		\
		$(O)/p_pspr.o			\
		$(O)/p_setup.o		\
		$(O)/p_sight.o		\
		$(O)/p_spec.o			\
		$(O)/p_switch.o		\
		$(O)/p_mobj.o			\
		$(O)/p_telept.o		\
		$(O)/p_tick.o			\
		$(O)/p_saveg.o		\
		$(O)/p_user.o			\
		$(O)/r_bsp.o			\
		$(O)/r_data.o			\
		$(O)/r_draw.o			\
		$(O)/r_main.o			\
		$(O)/r_plane.o		\
		$(O)/r_segs.o			\
		$(O)/r_sky.o			\
		$(O)/r_things.o		\
		$(O)/w_wad.o			\
		$(O)/wi_stuff.o		\
		$(O)/v_video.o		\
		$(O)/st_lib.o			\
		$(O)/st_stuff.o		\
		$(O)/hu_stuff.o		\
		$(O)/hu_lib.o			\
		$(O)/s_sound.o		\
		$(O)/z_zone.o			\
		$(O)/info.o				\
		$(O)/sounds.o
		

all:	 $(O)/paranutdoom

clean:
	rm -f *.o *~ *.flc
	rm -f $(O)/*

$(O)/paranutdoom:	$(O) $(OBJS) $(O)/i_main.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_main.o \
	-o $(O)/paranutdoom $(LIBS)

$(O)/%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O)/%.o:	vears/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(O): 
	mkdir $@

.PHONY: bin
bin: $(O)/paranutdoom.bin
$(O)/paranutdoom.bin: $(O)/paranutdoom
	riscv64-unknown-elf-objcopy -S -O binary $< $@

.PHONY: flash-%
flash-%: bin
	paranut_flash -c -p $(O)/paranutdoom.bin $(SYSTEMS_DIR)/$*/hardware/build/system.hdf $(SYSTEMS_DIR)/$*/hardware/firmware/firmware.elf
	
flash-%-bit: bin
	paranut_flash -c -b $(SYSTEMS_DIR)/$*/hardware/build/system.bit -p $(O)/paranutdoom.bin \
	                               $(SYSTEMS_DIR)/$*/hardware/build/system.hdf $(SYSTEMS_DIR)/$*/hardware/firmware/firmware.elf

#############################################################
#
#############################################################
