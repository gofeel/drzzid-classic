DOCKER ?= docker
IMAGE ?= drzzid:legacy
BBSACCOUNT ?= $(shell id -un)
.DEFAULT_GOAL := build
.PHONY: build test install clean docker-build docker-run
build:
	$(MAKE) -C src all BBSACCOUNT=$(BBSACCOUNT)
test: build
	$(MAKE) -C src check
install: build
	$(MAKE) -C src install BBSACCOUNT=$(BBSACCOUNT)
clean:
	$(MAKE) -C src clean
docker-build:
	$(DOCKER) build -t $(IMAGE) .
docker-run: docker-build
	$(DOCKER) run --rm -it $(IMAGE)
