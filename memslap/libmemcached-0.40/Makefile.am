ACLOCAL_AMFLAGS = -I m4

# includes append to these:
SUFFIXES =
PHONY =
TESTS =
CLEANFILES =
bin_PROGRAMS =
noinst_HEADERS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
noinst_PROGRAMS =
include_HEADERS =
nobase_include_HEADERS =
EXTRA_HEADERS =
BUILT_SOURCES=
EXTRA_DIST = README.FIRST

SUBDIRS = docs

test-docs:
	(cd docs && $(MAKE) test-docs)
include libmemcached/include.am
include clients/include.am
include libhashkit/include.am
include tests/include.am
include example/include.am
include support/include.am

check-local: test-no-outputdiff


fedora:
	rm -f ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm
	rm -f ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm
	cp libmemcached-$(VERSION).tar.gz ~/rpmbuild/SOURCES/
	rpmbuild -ba support/libmemcached.spec
	cp ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm .
	cp ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm .

generic:
	rm -f ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm
	rm -f ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm
	cp libmemcached-$(VERSION).tar.gz ~/rpmbuild/SOURCES/
	rpmbuild -ba support/libmemcached.spec
	cp ~/rpmbuild/RPMS/x86_64/libmemcached-$(VERSION)*.rpm .
	cp ~/rpmbuild/SRPMS/libmemcached-$(VERSION)*.rpm .

rpm: all dist generic fedora

merge-clean:
	find ./ | $(GREP) \.gcda | xargs rm -f
	find ./ | $(GREP) \.gcno | xargs rm -f
	find ./ | $(GREP) \.gz | xargs rm -f
	find ./ | $(GREP) \.orig | xargs rm -f
	find ./ | $(GREP) \.rej | xargs rm -f
	find ./ | $(GREP) \.rpm | xargs rm -f
	find ./ | $(GREP) \~$$ | xargs rm -f
	bzr unknowns

lcov: lcov-clean check
	@echo
	@echo "	------------------------------------------------------"
	@echo "Make sure ./configure was run with '--enable-coverage'"
	@echo "------------------------------------------------------"
	@echo
	cd libmemcached && lcov --capture --directory . --base-directory .. --output-file lcov.out
	genhtml -o lcov --function-coverage -t libmemcached libmemcached/lcov.out

lcov-clean: clean
	rm -rf lcov */lcov.out 
	find . -name '*.gcno' | xargs rm -f
	find . -name '*.gcda' | xargs rm -f



