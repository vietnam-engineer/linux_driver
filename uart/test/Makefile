WORK_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

all: run-cmake

run-cmake:
	@echo $(WORK_DIR)
	mkdir -p $(WORK_DIR)/build
	cmake -S $(WORK_DIR) -B $(WORK_DIR)/build
	make -C $(WORK_DIR)/build all

clean:
	rm -rf $(WORK_DIR)/build
