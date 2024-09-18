CXXFLAGS=-g $(shell pkg-config --cflags libpressio_cxx)
LDLIBS=$(shell pkg-config --libs libpressio_cxx)
LIBNAME ?= base
ALL=libpressio_typeswitch_base.so libpressio_typeswitch_solution.so test_pressio_base
all:$(ALL)
lib%.so:%.cc
	$(CXX) $(CXXFLAGS) -fPIC -shared -o $@ $^
test_pressio_base: libpressio_typeswitch_$(LIBNAME).so
test_pressio_base: LDFLAGS+=-Wl,-rpath=$(shell pwd) -L$(shell pwd) -l pressio_typeswitch_$(LIBNAME)
check: $(ALL)
	./test_pressio_base
clean:
	$(RM) $(ALL)
