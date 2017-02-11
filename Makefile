

LN ?= en

# source files
PROJ = parse_csv filter test modify conv_utf conv_ascii dump_map

PROJ_EXE = $(foreach d,$(PROJ),build/$(d).exe)

# binaries
EXE_PARSE_CSV = build/parse_csv.exe
EXE_FILTER    = build/filter.exe
EXE_TEST      = build/test.exe
EXE_MODIFY    = build/modify.exe
EXE_CONV_UTF  = build/conv_utf.exe
EXE_CONV_ASCII= build/conv_ascii.exe
EXE_DUMP_MAP  = build/dump_map.exe

##

all: help

help:
	@echo Valid targets are: filter, test, show, update, clean, map
	@echo the LN variable defines the language to be used

update: clean
	make filter LN=sv
	make filter LN=en
	make filter LN=it
	make filter LN=ru

##

build/$(LN).mod: build/$(LN).raw $(EXE_MODIFY)
	$(EXE_MODIFY) -i build/$(LN).raw -o build/$(LN).mod -a data/$(LN).add -r data/$(LN).remove


build/$(LN).bin: build/$(LN).mod $(EXE_FILTER)
	$(EXE_FILTER) -m data/$(LN).map -i build/$(LN).mod -c data/$(LN).config -o build/$(LN).bin

filter: build/$(LN).bin
	ls -l build/$(LN).*

# language-specific stuff

build/ru.raw: data/ru.utf8 $(EXE_CONV_UTF)
	$(EXE_CONV_UTF) data/ru.utf8 data/ru.map build/ru.tmp
	sort build/ru.tmp | uniq > $@

build/en.raw: data/en.txt
	sort data/en.txt | uniq > $@

build/sv.raw: data/sv.csv $(EXE_PARSE_CSV) $(EXE_CONV_ASCII)
	$(EXE_CONV_ASCII) data/sv.csv data/sv.map2 build/sv.tmp
	$(EXE_PARSE_CSV) < build/sv.tmp > $@

build/it.raw: data/it.txt
	sort data/it.txt | uniq > $@

##

# the tools

build/%.exe: src/%.c src/common.c src/common.h build
	gcc -O2 $< src/common.c -o $@

test: $(EXE_TEST) build/$(LN).bin
	$(EXE_TEST) build/$(LN).bin


map: build/$(LN).bin $(EXE_DUMP_MAP)
	$(EXE_DUMP_MAP) build/$(LN).bin


##
build:
	mkdir build

clean:
	-rm -rf build
