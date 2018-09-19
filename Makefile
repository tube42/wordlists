
LANG ?= en

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
	@echo Valid targets are: filter, test, update, clean, map
	@echo the LANG variable defines the language to be used

update: clean
	make filter LANG=sv
	make filter LANG=en
	make filter LANG=it
	make filter LANG=ru
	make filter LANG=fr

##

build/$(LANG).mod: $(EXE_MODIFY) build/$(LANG).raw
	$(EXE_MODIFY) -i build/$(LANG).raw -o build/$(LANG).mod -a data/$(LANG).add -r data/$(LANG).remove


build/$(LANG).bin: $(EXE_FILTER) build/$(LANG).mod
	$(EXE_FILTER) -m data/$(LANG).map -i build/$(LANG).mod -c data/$(LANG).config -o build/$(LANG).bin

filter: $(EXE_FILTER) build/$(LANG).bin
	ls -l build/$(LANG).*

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

build/fr.raw: data/fr.txt
	sort data/fr.txt | uniq > $@
	
##

# the tools

build/%.exe: src/%.c src/common.c src/common.h build
	gcc -O2 $< src/common.c -o $@

test: $(EXE_TEST) build/$(LANG).bin
	$(EXE_TEST) build/$(LANG).bin


map: build/$(LANG).bin $(EXE_DUMP_MAP)
	$(EXE_DUMP_MAP) build/$(LANG).bin


##
build:
	mkdir build

clean:
	-rm -rf build
