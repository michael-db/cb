.PHONY: test clean
test: cb
	@export DIGITS=     # clean default test environment
	@ls test/*.test|xargs -n 1 test/run-test

cb: cb.c
	@VERSION="$$(./get-version.sh)"; \
	echo "Making $@, version=\"$${VERSION}\""; \
	sed 's|// VERSION|"\\nVersion: '"$${VERSION}"'\\n"|' $^ | \
	cc -o $@ -x c - -Wall -Wextra -std=c99 -pedantic \
	-Wmissing-prototypes -Wstrict-prototypes -O2 -Wconversion

install: test
	cp cb /usr/bin

README.html: README.md
	pandoc -f gfm -o $@ $^

clean:
	rm -rf cb README.html test/*.out test/*.err
	find test -empty -type f -delete
