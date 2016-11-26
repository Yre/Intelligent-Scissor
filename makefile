################ template makefile ##############
# We don't know what compiler to use to build fltk on this machine - but fltk-config does...
CXX = $(shell fltk-config --cxx)

# Set the flags for compiler: fltk-config knows the basic settings, then we can add our own...
CXXFLAGS = $(shell fltk-config --cxxflags) -Wall -O3 
#-I/other/include/paths...

# We don't know what libraries to link with: fltk-config does...
LINKFLTK = $(shell fltk-config --ldstaticflags)
LINKFLTK_GL = $(shell fltk-config --use-gl --ldstaticflags)
LINKFLTK_IMG = $(shell fltk-config --use-images --ldstaticflags)

# Possible steps to run after linking...
STRIP      = strip
POSTBUILD  = fltk-config --post # Required on OSX, does nothing on other platforms, so safe to call


# Define what your target application is called
all: MyApp

# Define how to build the various object files...

main.o: src/main.cxx 
		$(CXX) -c $< $(CXXFLAGS)

bitmap.o: src/bitmap.cxx  
		$(CXX) -c $< $(CXXFLAGS)

ISDoc.o: src/ISDoc.cxx 
		$(CXX) -c $< $(CXXFLAGS)

ISUI.o: src/ISUI.cxx 
		$(CXX) -c $< $(CXXFLAGS)

PicView.o: src/PicView.cxx 
		$(CXX) -c $< $(CXXFLAGS)
		
# Now define how to link the final app - let's assume it needs image and OpenGL support
MyApp:  main.o bitmap.o ISDoc.o ISUI.o PicView.o
		$(CXX) -o $@ main.o bitmap.o ISDoc.o ISUI.o PicView.o $(LINKFLTK_GL)
		$(STRIP) $@
		$(POSTBUILD) $@  # only required on OSX, but call it anyway for portability
		rm -f *.o

clean: 
	rm -fr *.o MyApp MyApp.app
############### end #################