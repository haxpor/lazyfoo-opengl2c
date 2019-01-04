PROGRAM=loadshadersrc
OUTPUT=loadshadersrc

CC = gcc
EXE = .out
FDIR = foundation
GLDIR = gl
# use -DGL_SILENCE_DEPRECATION to silence deprecation warnings as we know we use older version of opengl
# you might want to remove it
# use -DDISABLE_SDL_TTF_LIB to disable code using SDL2_ttf
#
# we use -DDISABLE_FPS_CALC here to disable rendering framerarte as this sample
# enable MSAA in which it doesn't play well with alpha blending
override CFLAGS += -std=c99 -Wall -I. -I/usr/local/include/SDL2 -I/Volumes/Slave/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/OpenGL.framework/Headers -I/usr/local/include/GL -DGL_SILENCE_DEPRECATION -I/usr/local/include/freetype2 -DDISABLE_SDL_TTF_LIB -DDISABLE_FPS_CALC
override LIBS += -lSDL2 -lSDL2_image -lSDL2_mixer -framework OpenGL -lGLEW -lfreetype
TARGETS = \
	  $(FDIR)/common.o \
	  $(FDIR)/krr_math.o \
	  $(FDIR)/LWindow.o \
	  $(FDIR)/LTexture.o \
	  $(FDIR)/LTimer.o \
	  $(FDIR)/vector.o \
	  $(GLDIR)/gl_util.o \
	  $(GLDIR)/gl_LTexture.o \
	  $(GLDIR)/gl_LSpritesheet.o \
	  $(GLDIR)/gl_LFont.o \
	  $(GLDIR)/gl_LShaderProgram.o \
	  $(GLDIR)/gl_LPlainPolygonProgram2D.o \
	  usercode.o \
	  $(PROGRAM).o \
	  $(OUTPUT)

# targets for linking (just not include $(OUTPUT)
TARGETS_LINK = $(filter-out $(OUTPUT),$(TARGETS))

.PHONY: all clean

all: $(TARGETS) 
	
$(OUTPUT): $(TARGETS_LINK)
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

$(FDIR)/vector.o: $(FDIR)/vector.c $(FDIR)/vector.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_util.o: $(GLDIR)/gl_util.c $(GLDIR)/gl_util.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LTexture.o: $(GLDIR)/gl_LTexture.c $(GLDIR)/gl_LTexture.h $(GLDIR)/gl_LTexture_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LSpritesheet.o: $(GLDIR)/gl_LTexture_spritesheet.c $(GLDIR)/gl_LTexture_spritesheet.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LFont.o: $(GLDIR)/gl_LFont.c $(GLDIR)/gl_LFont.h $(GLDIR)/gl_LFont_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LShaderProgram.o: $(GLDIR)/gl_LShaderProgram.c $(GLDIR)/gl_LShaderProgram.h $(GLDIR)/gl_LShaderProgram_internals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(GLDIR)/gl_LPlainPolygonProgram2D.o: $(GLDIR)/gl_LPlainPolygonProgram2D.c $(GLDIR)/gl_LPlainPolygonProgram2D.h
	$(CC) $(CFLAGS) -c $< -o $@

usercode.o: usercode.c usercode.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGRAM).o: $(PROGRAM).c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf foundation/*.o
	rm -rf gl/*.o
	rm -rf *.out *.o *.dSYM
