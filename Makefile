################################################################################
# Encoding: UTF-8                                                  Tab size: 4 #
#                                                                              #
#                         GNU MAKEFILE FOR STOCKFILTER                         #
#                                                                              #
# License: LGPLv3+                               Copyleft (Ɔ) 2014, Jack Black #
################################################################################

#******************************************************************************#
#       Installation directories                                               #
#******************************************************************************#

# Project directories
incdir			:= include
srcdir			:= source

# System directories
prefix			:= /usr
exec_prefix		:= $(prefix)
sysconfdir		:= $(prefix)/etc
includedir		:= $(prefix)/include
datarootdir		:= $(prefix)/share
localstatedir	:= $(prefix)/var
sbindir			:= $(exec_prefix)/sbin
bindir			:= $(exec_prefix)/bin
libdir			:= $(exec_prefix)/lib
datadir			:= $(datarootdir)
appdir			:= $(datarootdir)/applications
localedir		:= $(datarootdir)/locale
icondir			:= $(datarootdir)/icons
infodir			:= $(datarootdir)/info
docdir			:= $(datarootdir)/doc/pkg_name
mandir			:= $(datarootdir)/man
man1dir			:= $(mandir)/man1
man2dir			:= $(mandir)/man2
man3dir			:= $(mandir)/man3
man4dir			:= $(mandir)/man4
man5dir			:= $(mandir)/man5
man6dir			:= $(mandir)/man6
man7dir			:= $(mandir)/man7
man8dir			:= $(mandir)/man8
htmldir			:= $(docdir)
pdfdir			:= $(docdir)
dvidir			:= $(docdir)
psdir			:= $(docdir)
man1ext			:= .1
man2ext			:= .2
man3ext			:= .3
man4ext			:= .4
man5ext			:= .5
man6ext			:= .6
man7ext			:= .7
man8ext			:= .8

#******************************************************************************#
#       Utility configuration                                                  #
#******************************************************************************#

# Utility names
CXX				:= g++
INSTALL			:= install

# Utility flags
CXXFLAGS		:= -I $(incdir) -Wall -std=gnu++11 -O2 -march=native -mtune=native -fomit-frame-pointer -llinasm -lcurl `pkg-config --cflags --libs gtk+-3.0`
INSTALLFLAGS	:=

#******************************************************************************#
#       Makefile variables                                                     #
#******************************************************************************#
vpath	%.h		$(incdir)
vpath	%.cpp	$(srcdir)

INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA	:= $(INSTALL) -m 644

program			:= stockfilter
theme			:= hicolor
size			:= scalable
icon			:= stock-filter.svg
desktop			:= stockfilter.desktop
objects			:= $(notdir $(patsubst %.cpp, %.o, $(wildcard $(srcdir)/*.cpp)))
dependencies	:= $(objects:.o=.d)

#******************************************************************************#
#       Makefile targets                                                       #
#******************************************************************************#
.SUFFIXES:
.PHONY: install-strip uninstall clean

all: $(program)

$(program): $(objects)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) -M $< -o $*.d

install: $(program)
	$(INSTALL_PROGRAM) -Dp $(INSTALLFLAGS) $(program) $(DESTDIR)$(bindir)/$(program)
	$(INSTALL_DATA) -Dp $(INSTALLFLAGS) $(icon) $(DESTDIR)$(icondir)/$(theme)/$(size)/apps/$(icon)
	$(INSTALL_DATA) -Dp $(INSTALLFLAGS) $(desktop) $(DESTDIR)$(appdir)/$(desktop)

install-strip:
	$(MAKE) install INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s'

uninstall:
	-cd $(DESTDIR)$(bindir) && rm -f $(program)
	-cd $(DESTDIR)$(icondir)/$(theme)/$(size)/apps && rm -f $(icon)
	-cd $(DESTDIR)$(appdir) && rm -f $(desktop)

clean:
	-rm -f $(program) $(objects) $(dependencies)

#******************************************************************************#
#       Dependency files                                                       #
#******************************************************************************#
-include $(dependencies)

################################################################################
#                                 END OF FILE                                  #
################################################################################
