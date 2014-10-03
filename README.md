PFO_Tools
=========

Tools to help plan item creation, training, and achievement farming for Pathfinder Online


I'll need to consume data from an xls spreadsheet.

The first thing I did after creating this github repo was look for a lib to parse xls.

    cd ~/Downloads
    wget http://downloads.sourceforge.net/libxls/libxls-0.2.0.tar.gz
    tar xzvf libxls-0.2.0.tar.gz
    cd libxls-0.2.0/
    cat INSTALL
    ./configure
    make
    sudo make install
    
    Libraries have been installed in:
   /usr/local/libxls/lib

   If you ever happen to want to link against installed libraries
   in a given directory, LIBDIR, you must either use libtool, and
   specify the full pathname of the library, or use the `-LLIBDIR'
   flag during linking and do at least one of the following:
      - add LIBDIR to the `LD_LIBRARY_PATH' environment variable during execution
      - add LIBDIR to the `LD_RUN_PATH' environment variable during linking
      - use the `-Wl,--rpath -Wl,LIBDIR' linker flag
      - have your system administrator add LIBDIR to `/etc/ld.so.conf'

