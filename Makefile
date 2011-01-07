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

OBJS=main.o pnglite.o graphic.o ship.o ai.o star.o particle.o network.o

# All Target
all: starc server

# Tool invocations
starc: $(OBJS)
	gcc -Wall -lglut -lGLU -lz -L. -lgrapple -o"starc" $(OBJS)
	@echo ' '

server: server.o
	gcc -Wall -L. -lgrapple -o"starc_server" server.o
	
%.o: %.c
	@echo 'CC: $<'
	@gcc -O0 -Wall -g -I../libgrapple-0.9.8/src/ -c -o"$@" "$<"

# Other Targets
clean:
	-$(RM) $(OBJS) 
	-$(RM) server.o starc_server
	-$(RM) starc
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
