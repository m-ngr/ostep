.PHONY: all clean c go test create-tests

# Default target runs the C build + create-tests
all: create-tests c

# ------------------------------------------------------------------------------
# Build Targets
# ------------------------------------------------------------------------------


create-tests: create-tests.c
	gcc -o create-tests create-tests.c -Wall -Werror -O

# Target: make c
c:
	gcc -o xcheck src/xcheck.c -Wall -Werror -O

# Target: make go
go:
	cd go && go build -o ../xcheck

# ------------------------------------------------------------------------------
# Testing Targets
# ------------------------------------------------------------------------------

# Target: make test c OR make test go OR make test (defaults to c)
test:
	@$(MAKE) clean
	@ARG="$(filter-out $@,$(MAKECMDGOALS))"; \
	TARGET=$${ARG:-c}; \
	echo "--> Building and testing ($$TARGET)..."; \
	$(MAKE) $$TARGET create-tests && ./create-tests && ./test-xcheck.sh

# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------

clean:
	rm -f xcheck create-tests tests/*.img
	rm -rf tests-out

# Catch-all rule so argument targets (like 'c' or 'go' after 'test') don't throw errors
%:
	@: