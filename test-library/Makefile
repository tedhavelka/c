##======================================================================
##
##  FILE:  Makefile for practice C libraries work
##
##
##  REFERENCES:
##
##   *  http://stackoverflow.com/questions/99132/how-to-prevent-directory-already-exists-error-in-a-makefile-when-using-mkdir
##
##
##
##======================================================================



export PROJECT_NAME=practice_c_libraries

export LIBRARY_NAME__BASENAME=libtestlib

export LIBRARY_NAME=libtestlib-$(PRACTICE_C_LIBRARY_VERSION__MAJOR_NUMBER)p$(PRACTICE_C_LIBRARY_VERSION__MINOR_NUMBER)

export PRACTICE_C_LIBRARY_VERSION__MAJOR_NUMBER=0

export PRACTICE_C_LIBRARY_VERSION__MINOR_NUMBER=1



OBJECT_FILES=diagnostics.o functions-string.o

HEADERS=diagnostics.h functions-string.h

SHARED_OBJECT_FILE=$(LIBRARY_NAME__BASENAME)-$(PRACTICE_C_LIBRARY_VERSION__MAJOR_NUMBER)p$(PRACTICE_C_LIBRARY_VERSION__MINOR_NUMBER).so

TARGETS=$(SHARED_OBJECT_FILE)



all: $(TARGETS)
#	$(CC) -shared -o $< $(OBJECT_FILES)



## TARGETS: $(OBJECT_FILES)
## libtestlib-$(PRACTICE_C_LIBRARY_VERSION__MAJOR_NUMBER)p$(PRACTICE_C_LIBRARY_VERSION__MINOR_NUMBER).so: $(OBJECT_FILES)
libtestlib-$(PRACTICE_C_LIBRARY_VERSION__MAJOR_NUMBER)p$(PRACTICE_C_LIBRARY_VERSION__MINOR_NUMBER).so: $(OBJECT_FILES)
	$(CC) -shared -o $@ $(OBJECT_FILES)



*.o: *.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $*.c -o $@





install:
	sudo mkdir -p /usr/local/include/$(LIBRARY_NAME)
	sudo cp -p $(HEADERS) /usr/local/include/$(LIBRARY_NAME)
	sudo mkdir -p /usr/local/lib/$(LIBRARY_NAME)
	sudo cp -p $(SHARED_OBJECT_FILE) /usr/local/lib/$(LIBRARY_NAME)



clean:
	rm -f ./*.o ./*.so






## --- EOF ---
