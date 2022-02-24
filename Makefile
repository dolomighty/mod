

PRE:=$(shell ./mk-headers.sh)
PRE:=$(shell ./mk-shell.sh)
SRC:=$(shell find -type f -name "*.c" -or -name "*.cpp" -or -name "*.asm")
HDR:=$(shell find -type f -name "*.h" -or -name "*.hpp")
RES:=$(shell find -type f -name "*.png")
DIR:=$(shell find -L -mindepth 1 -type d -not -wholename "*/.*" -printf "-I %P ")




CC=g++
LIBS  = `pkg-config --libs   sdl2` -lm
CFLAGS= `pkg-config --cflags sdl2` -Werror $(DIR)

# optim
CFLAGS+=-O3



.PHONY : all
all : main



.PHONY : run
run : main
	./$^




OBS+=main.o
#OBS+=mod.o

$(OBS) : $(SRC) $(HDR)


main : $(OBS)
	$(CC) $(CFLAGS) -o $@ $(OBS) $(LIBS)


#DYN+=draw_scene_gl.h
#draw_scene_gl.h : scenes/scene.obj obj2c.sh
#	./obj2c.sh scenes/scene.obj > $@


.PHONY : clean cl
clean cl :
	file * | awk '/ELF/ { gsub(/:.*/,"") ; print }' | xargs -r rm
	rm -fR deps.inc dyn



.PHONY : rebuild re
rebuild re : clean all



