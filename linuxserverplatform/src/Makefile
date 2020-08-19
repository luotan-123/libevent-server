# Top level makefile

build_ver=debug
export build_ver

default: all

.DEFAULT:
	cd Kernel && $(MAKE) 
	cd Common && $(MAKE) 
	cd CenterServer && $(MAKE) 
	cd LogonServer && $(MAKE) 
	cd WorkServer && $(MAKE) 
	cd GameManage && $(MAKE) 
	cd LoaderServer && $(MAKE) 
	cd httpserver && cmake ./ && $(MAKE) 
	cd games && $(MAKE) 
	cd test && $(MAKE) 

install:
	cd Kernel && $(MAKE) $@
	cd Common && $(MAKE) $@
	cd CenterServer && $(MAKE) $@
	cd LogonServer && $(MAKE) $@
	cd WorkServer && $(MAKE) $@
	cd GameManage && $(MAKE) $@
	cd LoaderServer && $(MAKE) $@
	cd games && $(MAKE) $@

clean:
	cd Kernel && $(MAKE) $@
	cd Common && $(MAKE) $@
	cd CenterServer && $(MAKE) $@
	cd LogonServer && $(MAKE) $@
	cd WorkServer && $(MAKE) $@
	cd GameManage && $(MAKE) $@
	cd LoaderServer && $(MAKE) $@
	cd httpserver && $(MAKE) $@
	cd games && $(MAKE) $@
	cd test && $(MAKE) $@

.PHONY: install
