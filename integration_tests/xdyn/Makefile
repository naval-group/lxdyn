.PHONY: all clean demos \
	invalid_yaml_returns_non_zero_error_code \
	tutorial_06 tutorial_12 \
	extra_hdf5_output_when_required \
	no_extra_hdf5_output_when_not_required \
	issue_196 issue_199 issue_202 issue_39 issue_235 \
	test_output_size

all: invalid_yaml_returns_non_zero_error_code \
	 tutorial_06\
	 tutorial_12\
	 extra_hdf5_output_when_required\
	 no_extra_hdf5_output_when_not_required\
	 issue_196\
	 issue_199\
	 issue_202\
	 issue_39\
	 issue_235\
	 test_output_size

# These used to run every step through `docker run … xdyn`, with the demo/tutorial inputs
# copied out of /usr/demos inside the image, and h5dump/python supplied by two more images.
# The zig lane builds static binaries with no runtime dependencies, so they run directly —
# and the same binaries that ship in the image also *generate* the demo inputs, so no image
# is needed at any step.
#
# Everything runs in $(WORK), outside the source tree: these tests write ~40 files and
# corrupt one of them on purpose. `clean` is then a single rm -rf, and there is no way to
# delete a tracked fixture by accident.
#
# Overridable, so the docker lane can still drive them as a cross-check:
#   make XDYN='docker run --rm -v $(PWD):/work -w /work xdyn'
# Derived from uname, not hardcoded: build.zig installs per target triple, so a fixed
# x86_64-linux-gnu points at nothing on aarch64 or macOS. See ../../tools/build-dir.sh.
BIN            ?= $(shell sh $(CURDIR)/../../tools/build-dir.sh)/bin
WORK           ?= $(abspath $(CURDIR)/../../build/scratch/integration-tests)
XDYN           ?= $(BIN)/xdyn
XDYN_FOR_CS    ?= $(BIN)/xdyn-for-cs
XDYN_FOR_ME    ?= $(BIN)/xdyn-for-me
GZ             ?= $(BIN)/gz
GENERATE_YAML  ?= $(BIN)/generate_yaml_example
GENERATE_STL   ?= $(BIN)/generate_stl_examples
H5DUMP         ?= h5dump
# check_output_size.py needs h5py + numpy, so the default is the venv uv builds from
# pyproject.toml's test-cli group -- not a bare `python3`, which under `nix develop` is now
# no interpreter at all and everywhere else is one with no h5py. `mise run integration`
# passes the same path explicitly; running make directly gets it too, or a clear ENOENT
# saying which venv is missing.
PYTHON         ?= $(abspath $(CURDIR)/../../build/venv)/bin/python

WRONG_ERROR_CODE=echo "Oops, didn't get the error code we were expecting: test failed!" && exit 1
EXPECTED_ERROR_CODE=echo "Got the error code we expected for an invalid YAML: success!"

# Tracked inputs the tests need; everything else in $(WORK) is generated.
FIXTURES = issue_196_test.yml issue_196_test.raodb.ini \
           issue_202.yml issue_202.py \
           issue_39.yml issue_39.csv issue_39.py \
           issue_235.yml cube_b.stl check_output_size.py

# Phony on purpose: invalid_yaml_returns_non_zero_error_code corrupts tutorial_01 with sed,
# so every run wants a fresh copy. Both generators together take well under a second.
demos:
	@mkdir -p $(WORK)
	@cp $(addprefix $(CURDIR)/,$(FIXTURES)) $(WORK)/
	@echo "Generating demo inputs into $(WORK)..."
	@cd $(WORK) && $(GENERATE_YAML) . && $(GENERATE_STL) .

issue_196: demos
	@echo "Attempting to use a Windows-generated PRECAL-R file with a Linux xdyn..."
	@cd $(WORK) && $(XDYN) issue_196_test.yml --dt 1 --tend 1
	@echo "Test passed!"

invalid_yaml_returns_non_zero_error_code: demos
	@echo "Corrupting tutorial_01_falling_ball.yml..."
	@# Not `sed -i`: GNU takes no argument, BSD/macOS takes the backup suffix.
	@cd $(WORK) && sed 's/rotations/rrrotations/g' tutorial_01_falling_ball.yml > corrupt.yml \
	    && mv corrupt.yml tutorial_01_falling_ball.yml
	@echo "Testing if this invalid YAML triggers a non-zero return code of xdyn..."
	@cd $(WORK) && { $(XDYN) --dt 1 --tend 1 tutorial_01_falling_ball.yml && ${WRONG_ERROR_CODE} || ${EXPECTED_ERROR_CODE}; }
	@cd $(WORK) && { $(XDYN_FOR_CS) --dt 1 -p 9002 tutorial_01_falling_ball.yml && ${WRONG_ERROR_CODE} || ${EXPECTED_ERROR_CODE}; }
	@cd $(WORK) && { $(XDYN_FOR_ME) -p 9002 tutorial_01_falling_ball.yml && ${WRONG_ERROR_CODE} || ${EXPECTED_ERROR_CODE}; }
	@cd $(WORK) && { $(GZ) --dphi=1 --phi_max=1 -s cube.stl tutorial_01_falling_ball.yml && ${WRONG_ERROR_CODE} || ${EXPECTED_ERROR_CODE}; }

include Makefile.check_exports
include Makefile.binary_stl
include Makefile.issue_202
include Makefile.issue_39
include Makefile.issue_235

# The one assertion the French manual's build made that nothing else here covers. Its
# Makefile ran this exact command through the doc image and failed on any "Warning" in the
# output; deleting docs/user_fr/ deletes that lane, so the check moves rather than dies.
#
# It is worth moving because tutorial_06 is the only CLI propulsion-with-commands path under
# test: a wageningen B-series propeller, a resistance curve and a `commands` section. The
# gRPC suite covers a controller, and the rest of this directory is issue regressions.
#
# Grepping for "Warning" is a usable assertion only because xdyn is otherwise completely
# silent on this input -- exit 0, nothing on stdout, nothing on stderr. Both streams are
# captured, where the original watched stdout alone.
tutorial_06: demos
	@echo "Running the propulsion tutorial, failing on any warning..."
	@cd $(WORK) && $(XDYN) tutorial_06_propulsion.yml --dt 0.1 --tend 1 > propulsion.log 2>&1
	@cd $(WORK) && if grep -q "Warning" propulsion.log; then \
	    echo "xdyn warned while running the propulsion tutorial:" && cat propulsion.log && exit 1; \
	else echo "Propulsion tutorial ran with no warnings!"; fi

tutorial_12: demos
	@echo "Launching tutorial 12..."
	@cd $(WORK) && $(XDYN) --dt 1 --tend 1 -o tsv tutorial_12_precal_r.yml
	@echo "All good!"

clean:
	@rm -rf $(WORK)
