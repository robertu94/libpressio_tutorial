CXXFLAGS=-g $(shell pkg-config --cflags libpressio_cxx)
LDLIBS=$(shell pkg-config --libs libpressio_cxx)
LIBNAME ?= base
ALL=test_runlength libpressio_runlength_base.so libpressio_runlength_solution.so test_pressio_base
all:$(ALL)
lib%.so:%.cc
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $@ $^
test_pressio_base: libpressio_runlength_$(LIBNAME).so
test_pressio_base: LDFLAGS+=-Wl,-rpath=$(shell pwd) -L$(shell pwd) -l pressio_runlength_$(LIBNAME)
check: $(ALL)
	./test_runlength
	./test_pressio_base
clean:
	$(RM) $(ALL)
