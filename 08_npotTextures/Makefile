PROGRAM=npottextures
OUTPUT=npottextures

CC = gcc
EXE = .out
FDIR = foundation
GLDIR = gl
override CFLAGS += -std=c99 -Wall -I. -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL
override LIBS += -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -framework OpenGL -lGLEW
TARGETS = \
	  $(FDIR)/common.o \
	  $(FDIR)/krr_math.o \
	  $(FDIR)/LWindow.o \
	  $(FDIR)/LTexture.o \
	  $(FDIR)/LTimer.o \
	  $(GLDIR)/gl_util.o \
	  $(GLDIR)/gl_LTexture.o \
	  usercode.o \
	  $(PROGRAM).o \
	  $(OUTPUT)

.PHONY: all clean

all: $(TARGETS) 

$(OUTPUT): $(PROGRAM).o $(FDIR)/LWindow.o $(FDIR)/LTexture.o $(FDIR)/common.o $(FDIR)/krr_math.o $(FDIR)/LTimer.o $(GLDIR)/gl_util.o $(GLDIR)/gl_LTexture.o usercode.o
	$(CC) $^ -o $(OUTPUT)$(EXE) $(LIBS)

$(FDIR)/common.o: $(FDIR)/common.c $(FDIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/krr_math.o: $(FDIR)/krr_math.c $(FDIR)/krr_math.h $(FDIR)/Circle.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/LWindow.o: $(FDIR)/LWindow.c $(FDIR)/LWindow.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/LTexture.o: $(FDIR)/LTexture.c $(FDIR)/LTexture.h
	$(CC) $(CFLAGS) -c $< -o $@

$(FDIR)/LTimer.o: $(FDIR)/LTimer.c $(FDIR)/LTimer.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_util.o: $(GLDIR)/gl_util.c $(GLDIR)/gl_util.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LTexture.o: $(GLDIR)/gl_LTexture.c $(GLDIR)/gl_LTexture.h
	$(CC) $(CFLAGS) -c $< -o $@

usercode.o: usercode.c usercode.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM).o: $(PROGRAM).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf foundation/*.o
	rm -rf gl/*.o
	rm -rf *.out *.o *.dSYM
