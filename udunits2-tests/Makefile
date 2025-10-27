.ONESHELL:

REPO_ROOT ?= ..
LIBDIR := $(REPO_ROOT)/lib/.libs
INCDIR := $(REPO_ROOT)/lib

CFLAGS += -I$(INCDIR) -g -O2 -fPIC -Wall -Wextra -Wno-missing-field-initializers
LDFLAGS += -L$(LIBDIR) -Wl,-rpath,$(LIBDIR) -ludunits2

# Build three separate executables (each source has its own main())
BIN_API := api_basic api_custom api_debug

all:  $(BIN_API)
	@echo "== Running CLI regression suite =="
	@REPO_ROOT=$(REPO_ROOT) bash ./run.sh; CLI_EXIT=$$?; true
	@echo "== Running API tests =="
	@LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_basic; BASIC_EXIT=$$?; true
	# @LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_custom; CUSTOM_EXIT=$$?; true
	# @LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_debug; DEBUG_EXIT=$$?; true
	if [ $$CLI_EXIT -ne 0 ]; then echo "Some bash CLI tests failed! 😞"; else echo "All bash CLI tests are successful! 🎉"; fi
	if [ $$BASIC_EXIT -ne 0 ]; then echo "Some basic C tests failed! 😞\n"; else echo "All basic C tests are successful! 🎉\n"; fi

cli:
	@echo "== Running CLI regression suite =="
	@REPO_ROOT=$(REPO_ROOT) bash ./run.sh; CLI_EXIT=$$?; true
	if [ $$CLI_EXIT -ne 0 ]; then echo "Some bash tests failed! 😞\n"; else echo "All bash tests successful! 🎉\n"; fi

api: $(BIN_API)
	@echo "== Running API tests =="
	@LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_basic; BASIC_EXIT=$$?
	# @LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_custom; CUSTOM_EXIT=$$?
	# @LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH DYLD_LIBRARY_PATH=$(LIBDIR):$$DYLD_LIBRARY_PATH ./api_debug; DEBUG_EXIT=$$?
	if [ $$BASIC_EXIT -ne 0 ]; then necho "Some basic C tests failed! 😞\n"; else echo "All basic C tests are successful! 🎉\n"; fi

api_basic: src/udunits2_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

api_custom: src/udunits2_custom_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

api_debug: src/udunits2_debug_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BIN_API)

test: cli api
