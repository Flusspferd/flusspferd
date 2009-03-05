-include .localsettings.mak

.PHONY: all clean reconfigure
all: .configure
	$(PYTHON) ./waf -v 2>&1 | sed -e 's/^\.\.\//\.\//' -e 's/cxx://' \
-e 's/[0-9][0-9]:[0-9][0-9]:[0-9][0-9]//g'

.configure:
	$(PYTHON) ./waf configure
	@touch .configure

reconfigure:
	$(PYTHON) ./waf configure

install:
	$(PYTHON) ./waf install

clean:
	rm .configure
	$(PYTHON) ./waf clean
