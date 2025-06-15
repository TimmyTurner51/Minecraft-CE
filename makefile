NAME ?= MC3D
ICON ?= icon.png
DESCRIPTION ?= "Minecraft 3D vMay11.25 for TI-84 CE"
COMPRESSED ?= Yes
ARCHIVED ?= NO

CFLAGS ?= -Wall -Wextra -Oz
CXXFLAGS ?= -Wall -Wextra -Oz
ifndef CEDEV
$(error CEDEV environment path variable is not set)
endif

# Include the default CEdev makefile, but override its assembler
include $(CEDEV)/meta/makefile.mk

all: gfx bin/MC3D.8xp postgfx

gfx:
	if not exist bin mkdir bin
	cd src/gfx && convimg

postgfx: src/gfx/MCTILES.8xv
	@cmd /C "move src\\gfx\\MCTILES.8xv bin\\"