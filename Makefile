INC=
LIBS = ./lib/libcurl.so -lpthread
CXX = g++
CPP_FLAG = -std=c++11
PRG = mutiThreadDownloader
SRCDIR = ./src
SRC = $(SRCDIR)/*.cpp
OBJS = *.o

$(PRG): $(OBJS)
	$(CXX) $(CPP_FLAG) -o $@ $(OBJS) $(LIBS)
$(OBJS): $(SRC)
	$(CXX) $(CPP_FLAG) $(INC) -c $^
	
clean:
	rm -f $(OBJS) $(PRG)
