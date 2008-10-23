-include .localsettings.mak

.PHONY: all clean reconfigure
all: .configure
	$(PYTHON) ./waf -v 2>&1 | sed -e 's/^\.\.\//\.\//'

.configure:
	$(PYTHON) ./waf configure
	@touch .configure

reconfigure:
	$(PYTHON) ./waf configure

clean:
	rm .configure
	$(PYTHON) ./waf clean
