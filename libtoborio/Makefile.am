#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libtoborio.la

#######################################
# Build information for each library

# Sources for libtoborio
libtoborio_la_SOURCES = em_log.c em_io_raspi.c em_io.c em_common.c

# Linker options libTestProgram
libtoborio_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libtoborio_la_CPPFLAGS = -I$(top_srcdir)/include

