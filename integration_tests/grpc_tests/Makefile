# Each scenario used to be a docker-compose stack driven by its own Makefile. They are now
# shell scripts that start the same processes on localhost; see ../tools/scenario.sh for
# what compose was actually providing and why only the readiness poll survived.
#
# Overridable, so a scenario can be pointed at a different build or interpreter:
#   make BIN=$(PWD)/../../build/aarch64-linux-musl/bin waves
SCENARIOS = waves waves_server wave+force cosim model_exchange-1 model_exchange-2 \
            controller force

# Six of these share a name with a directory here, so without .PHONY make calls them up to
# date and skips them -- exiting 0 having run a third of the suite. SCENARIOS has to be
# defined before this line: .PHONY takes the value it has when the line is read.
.PHONY: all clean $(SCENARIOS)

all: $(SCENARIOS)

waves waves_server wave+force cosim controller force:
	@sh $@/run.sh

model_exchange-1:
	@sh model_exchange/test-1/run.sh

model_exchange-2:
	@sh model_exchange/test-2/run.sh

clean:
	@rm -rf ../../build/scratch/integration-tests
