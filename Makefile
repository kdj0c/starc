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

CL_DIR:=cl_obj
CL_OBJS=$(addprefix $(CL_DIR)/,main.o pnglite.o graphic.o ship.o ai.o star.o particle.o network.o menu.o gamemain.o config.o turret.o mothership.o)
CL:=starc
SV_DIR:=sv_obj
SV_OBJS=$(addprefix $(SV_DIR)/,server.o ship.o network.o ai.o config.o turret.o mothership.o)
SV:=ded_starc

# All Target
all: $(CL) $(SV)

# Tool invocations
$(CL): $(CL_OBJS)
	gcc -Wall -lglut -lGLU -lz -lm -L. -lgrapple -lftgl -lconfig -o"$(CL)" $(CL_OBJS)
	@echo ' '

$(SV): $(SV_OBJS)
	gcc -Wall -L. -lgrapple -lm -lconfig -o"$(SV)" $(SV_OBJS)

$(CL_OBJS): | $(CL_DIR)

$(CL_DIR):
	mkdir $(CL_DIR)
	
$(SV_OBJS): | $(SV_DIR)

$(SV_DIR):
	mkdir $(SV_DIR)
	
$(CL_DIR)/%.o : %.c
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -I/usr/include/freetype2 -c -o"$@" "$<"
	
$(SV_DIR)/%.o : %.c
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -DDEDICATED -c -o"$@" "$<"
     
# Other Targets
clean:
	-$(RM) $(CL_OBJS) $(CL) 
	-$(RM) $(SV_OBJS) $(SV)
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
