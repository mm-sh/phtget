CXXFLAGS = -Wall -Werror -Wextra -std=c++11 -pedantic
CXX = g++

all: options phtget

options:
	@echo phtget build options
	@echo "CXXFLAGS	= ${CXXFLAGS}"
	@echo "CXX		= ${CXX}"

.cc.o:
	${CXX} -c ${CXXFLAGS} $<

phtget: phtget.o Downloader.o Parser.o
	$(CXX) -o $@ $^

clean:
	rm -f phtget
	rm -f *.o
