.PHONY: all
all:
	$(MAKE) -C src all
	cp --update src/bmfs bmfs
	cp --update src/bmfs-fuse bmfs-fuse

.PHONY: clean
clean:
	$(MAKE) -C src clean

.PHONY: test
test:
	$(MAKE) -C src test

.PHONY: install
install:
	$(MAKE) -C src install

