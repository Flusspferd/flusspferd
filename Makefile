-include .localsettings.mak

.PHONY: all clean reconfigure distclean
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
	rm -f .configure
	$(PYTHON) ./waf clean

distclean:
	rm -f .configure
	$(PYTHON) ./waf distclean
