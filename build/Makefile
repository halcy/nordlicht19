# Base directory is one up
BASEDIR := ../

# Binary name, author name, desc
TARGET := nordlicht19
AUTHOR := SVatG
DESC := Raumfahrtaufgabenuebertragungsgesetz

# Working directories
BUILD := $(BASEDIR)/build
SOURCES := $(BASEDIR)/source $(BASEDIR)/source/VectorLibrary $(BASEDIR)/source/Rocket
DATA := $(BASEDIR)/data
#ASSETS := $(BASEDIR)/asset_source

# Libraries
LIBDIRS := $(DEVKITPRO)/libctru
LIBS := -lcitro3d -lctru -lm

# Vpath for out of tree build
VPATH := $(BUILD):$(SOURCES):$(DATA)

# Tools
CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
NM := arm-none-eabi-nm
LD := $(CC)

# Tool flags
ARCH := -g -march=armv6k -mtune=mpcore -mfloat-abi=hard -mword-relocations
CFLAGS := -Wno-missing-braces -O3 \
          -std=c99 \
	  -fomit-frame-pointer -ffunction-sections \
	  -mfloat-abi=hard -ffast-math -funsafe-math-optimizations \
	  $(ARCH) -DARM11 -D_3DS \
	  -DABGR32Pixels \
	  $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(BUILD)
ASFLAGS := 
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) \
	   $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

# Input files
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
PICAFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
#PNGFILES := $(foreach dir,$(ASSETS),$(notdir $(wildcard $(dir)/*.png)))

# Output file
OUTPUT := $(CURDIR)/$(TARGET)

OFILES := $(addsuffix .o,$(BINFILES)) \
	  $(PICAFILES:.v.pica=.shbin.o) \
	  $(SHLISTFILES:.shlist=.shbin.o) \
	  $(CFILES:.c=.o)

# Actual binary
all: $(OUTPUT).3dsx
$(OUTPUT).3dsx: $(OUTPUT).elf $(OUTPUT).smdh
$(OUTPUT).elf: $(OFILES)

clean:
	rm -fr $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf *.o *.h *.lst *.shbin *~

# Want GNU extended syntax
.SUFFIXES:

# Base compilation / assembly build rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) -c $< -o $@ 
	
# Basic "binary blob" build rule
%.bin.o: %.bin
	bin2s $< | $(AS) -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(<F) | tr . _)`.h

# GPU shader assembly.
define shader-as
	$(eval CURBIN := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $@
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
endef

%.shbin.o : %.v.pica %.g.pica
	$(call shader-as,$^)

%.shbin.o : %.v.pica
	$(call shader-as,$<)

%.shbin.o : %.shlist
	$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))

# App metainfo
%.smdh: $(APP_ICON) $(MAKEFILE_LIST)
	smdhtool --create "$(TARGET)" "$(DESC)" "$(AUTHOR)" $(BASEDIR)/icon.png $@

# Binary from elf build rule
%.3dsx: %.elf
	3dsxtool $< --smdh=$(BASEDIR)build/$(TARGET).smdh --romfs=$(BASEDIR)romfs/ $(_3DSXFLAGS) $@

# Linking rule
%.elf: 3dsx_crt0.o
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
	$(NM) -CSn $@ > $(notdir $*.lst)
