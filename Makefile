#compilateur
CC = g++

#compil en mode 'debug' ou optmisée (-O2)
DBG = yes

ifeq ($(DBG),yes) #en mode debug
  CFLAGS = -g -Wpointer-arith -Wall -ansi
else               #en mode normal
  CFLAGS = -O2 -ansi
endif

# assemblage des infos de lib. et inc.
lib = -lpthread
# fichiers *.c locaux
src = src/
# fichiers *.h locaux et lib.
inc = -I. -I./include

all : tracker peer clean
	mv tracker bin/
	mv peer bin/

tracker : tracker.o
	@echo "assemblage [$^]->$@"
	$(CC) $^ $(lib) -o $@
	@echo "------------------------"

peer : peer.o
	@echo "assemblage [$^]->$@"
	$(CC) $^ $(lib) -o $@
	@echo "------------------------"
	
tracker.o : $(src)tracker.cpp
	@echo "module $@"
	$(CC) $(CFLAGS) $(inc) -c $< -o $@
	@echo "------------------------"

peer.o : $(src)peer.cpp
	@echo "module $@"
	$(CC) $(CFLAGS) $(inc) -c $< -o $@
	@echo "------------------------"

.PHONY : clean cleanall ?

# informations sur les paramètres de compilation       
? :
	@echo "---------compilation informations----------"
	@echo "  processor      : $(PROCBIT)"
	@echo "  compiler       : $(CC)"
	@echo "  options        : $(CFLAGS)"
	@echo "  lib g2x/OpenGl : $(libG2X)$(COMP)"
	@echo "  headers        : $(incG2X)"
clean : 
	@rm -f *.o
cleanall :
	@rm -f *.o peer tracker
	
