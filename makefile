SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
LD_FLAGS = `pkg-config --libs OGRE OGRE-Terrain OIS` -lboost_system -lboost_thread-mt -lenet 
CC_FLAGS = -g `pkg-config --cflags OGRE OGRE-Terrain OIS` 
TITLE = tut4

$(TITLE): $(OBJS)
	g++ -o $@ $^ $(LD_FLAGS)
%.o: %.cpp
	g++ $(CC_FLAGS) -c $<

clean:
	rm *.o
	rm $(TITLE)

run:
	./$(TITLE)
