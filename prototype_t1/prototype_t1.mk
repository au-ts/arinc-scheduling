# In case the default compiler triple doesn't work for you or your package manager
# only has aarch64-none-elf or something, you can specifiy the toolchain.
ifndef TOOLCHAIN
	# Get whether the common toolchain triples exist
	TOOLCHAIN_AARCH64_LINUX_GNU := $(shell command -v aarch64-linux-gnu-gcc 2> /dev/null)
	TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU := $(shell command -v aarch64-unknown-linux-gnu-gcc 2> /dev/null)
	# Then check if they are defined and select the appropriate one
	ifdef TOOLCHAIN_AARCH64_LINUX_GNU
		TOOLCHAIN := aarch64-linux-gnu
	else ifdef TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU
		TOOLCHAIN := aarch64-unknown-linux-gnu
	else
		$(error "Could not find an AArch64 cross-compiler")
	endif
endif

TOOLCHAIN := aarch64-unknown-linux-gnu
CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
AR := $(TOOLCHAIN)-ar
RANLIB := $(TOOLCHAIN)-ranlib


TIMER_DRIVER := $(SDDF)/drivers/timer/$(TIMER_DRIVER_DIR)

include ${TIMER_DRIVER}/timer_driver.mk
include ${SDDF}/util/util.mk

IMAGES := p1_ppd.elf p1_spd.elf p2_ppd.elf p2_spd.elf p3_ppd.elf p3_spd.elf scheduler.elf timer_driver.elf
# Note that these warnings being disabled is to avoid compilation errors while in the middle of completing each exercise part
CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g -Wall -Wno-array-bounds -Wno-unused-variable -Wno-unused-function -Werror -I$(BOARD_DIR)/include -I$(SDDF)/include -Iinclude -DBOARD_$(BOARD)
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := -lmicrokit -Tmicrokit.ld libsddf_util_debug.a

IMAGE_FILE = loader.img
REPORT_FILE = report.txt


${IMAGES}: libsddf_util_debug.a




directories:
	$(info $(shell mkdir -p $(BUILD_DIR)))

qemu: $(IMAGE_FILE)
	qemu-system-aarch64 -machine virt,virtualization=on \
		-cpu $(CPU) \
		-serial mon:stdio \
		-device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0 \
		-m size=2G \
		-nographic \
		-netdev user,id=mynet0 \
		-device virtio-net-device,netdev=mynet0,mac=52:55:00:d1:55:01

%.o: ../%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

%.o: ../p1/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

%.o: ../p2/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

%.o: ../p3/%.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

PRINTF_OBJS := printf.o util.o
INTERPARTITION_COMM_OBJS := $(PRINTF_OBJS) interpartitioncomm.o 
P1_USER_OBJS := $(INTERPARTITION_COMM_OBJS) p1_user.o p1_ppd.o
P2_USER_OBJS := $(INTERPARTITION_COMM_OBJS) p2_user.o p2_ppd.o
P3_USER_OBJS := $(INTERPARTITION_COMM_OBJS) p3_user.o p3_ppd.o

P1_SPD_OBJS := $(INTERPARTITION_COMM_OBJS) p1_spd.o
P2_SPD_OBJS := $(INTERPARTITION_COMM_OBJS) p2_spd.o
P3_SPD_OBJS := $(INTERPARTITION_COMM_OBJS) p3_spd.o

PARTITION_OBJS := $(PRINTF_OBJS) partition.o
SCHEDULER_OBJS := $(PARTITION_OBJS) scheduler.o
SYSTEM_FILE := ${TOP}/prototype1.system

all: directories $(IMAGE_FILE)

# all: $(IMAGE_FILE)
# 	CHECK_FLAGS_BOARD_MD5:=.board_cflags-$(shell echo -- ${CFLAGS} ${BOARD} ${MICROKIT_CONFIG}| shasum | sed 's/ *-//')

# ${CHECK_FLAGS_BOARD_MD5}:
# 	-rm -f .board_cflags-*
# 	touch $@

# P1 #

p1_ppd.elf: $(P1_USER_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

p1_spd.elf: $(P1_SPD_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@


# P2 #

p2_ppd.elf: $(P2_USER_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

p2_spd.elf: $(P2_SPD_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

# P3 #

p3_ppd.elf: $(P3_USER_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

p3_spd.elf: $(P3_SPD_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@


scheduler.elf: $(SCHEDULER_OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@


$(IMAGE_FILE): $(IMAGES) $(SYSTEM_FILE)
	$(MICROKIT_TOOL) $(SYSTEM_FILE) --search-path $(BUILD_DIR) --board $(MICROKIT_BOARD) --config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)