# Copyright (C) Jocelyn Falempe jock@inpactien.com.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2 as
#  published by the Free Software Foundation.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

COMMON:=ship.o network.o ai.o config.o turret.o mothership.o event.o vec.o weapon.o gametime.o save.o

CL_DIR:=cl_obj
CL_OBJS=$(addprefix $(CL_DIR)/,$(COMMON) main.o pnglite.o graphic.o star.o particle.o menu.o gamemain.o)
CL:=starc
SV_DIR:=sv_obj
SV_OBJS=$(addprefix $(SV_DIR)/,$(COMMON))
SV:=ded_starc

DEP:=$(wildcard *.h)

# All Target
all: $(CL) $(SV)

# Tool invocations
$(CL): $(CL_OBJS)
	@echo 'LN: $(CL)'
	@gcc -Wall -L. -o"$(CL)" $(CL_OBJS) -lglut -lGLU -lz -lm -lgrapple -lftgl -lconfig

$(SV): $(SV_OBJS)
	@echo 'LN: $(SV)'
	@gcc -Wall -L. -o"$(SV)" $(SV_OBJS) -lgrapple -lm -lconfig -lrt

$(CL_OBJS): | $(CL_DIR)

$(CL_DIR):
	mkdir $(CL_DIR)

$(SV_OBJS): | $(SV_DIR)

$(SV_DIR):
	mkdir $(SV_DIR)

$(CL_DIR)/%.o : %.c $(DEP)
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -I/usr/include/freetype2 -c -o"$@" "$<"

$(SV_DIR)/%.o : %.c $(DEP)
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -DDEDICATED -c -o"$@" "$<"

# Other Targets
clean:
	-$(RM) $(CL_OBJS) $(CL)
	-$(RM) $(SV_OBJS) $(SV)
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
