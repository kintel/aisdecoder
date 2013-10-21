REPORTER = list

test:
	mocha --reporter $(REPORTER)

test-w:
	@NODE_ENV=test mocha \
	  --reporter $(REPORTER) \
	  --growl \
	  --watch

test-cov: lib-cov 
	@MOCHA_COV=1 $(MAKE) test REPORTER=html-cov > coverage.html
	@open coverage.html

lib-cov:
	@jscover src src-cov


.PHONY: test
