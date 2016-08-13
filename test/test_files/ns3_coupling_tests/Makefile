.PHONY: check
check: \
	bin/test-ieee39_relay_geco_coupled.tc

%.tc: clean $@
	@${PARGRID_SRC_DIR}/bin/test-runner $@


.PHONY: clean
clean:
	@rm -fr output/*
	@rm -f dynfault.csv *.tr
	@rm -f geco-flow.xml
