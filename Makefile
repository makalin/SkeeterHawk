# SkeeterHawk Makefile
# Builds both simulation and firmware

.PHONY: all sim firmware clean install-deps

all: sim firmware

# Python simulation
sim:
	@echo "Running Python simulation..."
	cd simulation && python3 sonar_sim.py

# Install Python dependencies
install-deps:
	@echo "Installing Python dependencies..."
	pip3 install -r simulation/requirements.txt

# Firmware build (requires STM32 toolchain)
firmware:
	@echo "Building firmware..."
	@if [ ! -d "firmware/build" ]; then mkdir -p firmware/build; fi
	cd firmware/build && cmake .. && make

clean:
	@echo "Cleaning build artifacts..."
	rm -rf firmware/build
	rm -rf simulation/__pycache__
	rm -f simulation/*.png
	find . -name "*.pyc" -delete
	find . -name "*.o" -delete
	find . -name "*.elf" -delete
	find . -name "*.bin" -delete
	find . -name "*.hex" -delete
	find . -name "*.map" -delete

