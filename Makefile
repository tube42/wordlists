
LANGUAGE ?= en

# source files
PROJ = publish test create map map_dump
PROJ_EXE = $(foreach d,$(PROJ),build/$(d).exe)

# binaries
EXE_PUBLISH   = build/publish.exe
EXE_TEST      = build/test.exe
EXE_CREATE    = build/create.exe
EXE_MAP       = build/map.exe
EXE_MAP_DUMP  = build/map_dump.exe

##

all: help

help:
	@echo Valid targets are: publish, test, update, clean, map
	@echo the LANGUAGE variable defines the langauge to be used

update: clean
	make publish LANGUAGE=sv
	make publish LANGUAGE=en
	make publish LANGUAGE=it
	make publish LANGUAGE=ru
	make publish LANGUAGE=fr
	make publish LANGUAGE=de

exe: build $(PROJ_EXE)
##

# this will create a single file to be added / removed
#
# note: tr has problem with non-ascii letters

build/$(LANGUAGE).add: data/$(LANGUAGE).add* $(EXE_MAP)
	cat data/$(LANGUAGE).add* | tr '[:upper:]' '[:lower:]' | sort  | uniq > build/tmp
	$(EXE_MAP) build/tmp data/$(LANGUAGE).map $@

build/$(LANGUAGE).remove: data/$(LANGUAGE).remove* $(EXE_MAP)
	cat data/$(LANGUAGE).remove* | tr '[:upper:]' '[:lower:]' | sort  | uniq > build/tmp
	$(EXE_MAP) build/tmp data/$(LANGUAGE).map $@

# from add & remove files create a single word list
build/$(LANGUAGE).txt: $(EXE_CREATE) build/$(LANGUAGE).add build/$(LANGUAGE).remove
	$(EXE_CREATE) -i build/$(LANGUAGE).add -r data/$(LANGUAGE).remove -o build/$(LANGUAGE).txt


# from *.txt word list, keep words of specified length and create a bin forma wordlist
build/$(LANGUAGE).bin: $(EXE_PUBLISH) build/$(LANGUAGE).txt
	$(EXE_PUBLISH) -m data/$(LANGUAGE).map -i build/$(LANGUAGE).txt -c data/$(LANGUAGE).config -o build/$(LANGUAGE).bin


##

# the tools

build/%.exe: src/%.c src/wordlist.* src/common.*
	mkdir -p build
	gcc -O2 -Wall $< src/wordlist.c src/common.c -o $@


map: build/$(LANGUAGE).bin $(EXE_MAP_DUMP)
	$(EXE_MAP_DUMP) build/$(LANGUAGE).bin

publish: $(EXE_PUBLISH) build/$(LANGUAGE).bin
	ls -l build/$(LANGUAGE).*

# examples

build/%.class: src/%.java
	mkdir -p build
	javac $< -d build

javatest: build/Wordlist.class build/$(LANGUAGE).bin
	java -cp build WordlistTest build/$(LANGUAGE).bin

test: $(EXE_TEST) build/$(LANGUAGE).bin
	$(EXE_TEST) build/$(LANGUAGE).bin

##
build:
	mkdir build

clean:
	-rm -rf build
