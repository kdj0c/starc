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

OBJS=main.o pnglite.o graphic.o ship.o

# All Target
all: starc

# Tool invocations
starc: $(OBJS)
	gcc -Wall -lglut -lGLU -lz -o"starc" $(OBJS) 
	@echo ' '
	
%.o: %.c
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -c -o"$@" "$<"

# Other Targets
clean:
	-$(RM) $(OBJS) 
	-$(RM) starc
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
