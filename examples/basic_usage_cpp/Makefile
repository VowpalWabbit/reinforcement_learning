TARGET = basic_usage_cpp.out

RL_LIB = -L ../../rlclientlib -lrlclient
VW_LIB = -L../../ext_libs/vowpal_wabbit/vowpalwabbit -lvw -lallreduce
BOOST_LIBS = -lboost_unit_test_framework -lboost_system
CPPREST_LIBS = -lcpprest -lssl -lcrypto -pthread -ldl
ALL_LIBS = $(RL_LIB) $(VW_LIB) $(BOOST_LIBS) $(CPPREST_LIBS) $(LIBS)

INCLUDE = -I ../../include -I ../../rlclientlib -I ../../ext_libs/vowpal_wabbit/explore -I ../../ext_libs

.PHONY: default all clean

default: $(TARGET)
all: default

things: all

SOURCES = $(wildcard *.cc)
OBJECTS = $(patsubst %.cc, %.o, $(SOURCES))
HEADERS = $(wildcard *.h)

%.o: %.cc $(HEADERS)
	$(CXX) $(FLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) $(LIBDIR) $(ALL_LIBS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
