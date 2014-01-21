SUBDIRS = memcached-1.2.2 memcached-1.2.3 memcached-1.2.4

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done
