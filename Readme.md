# A MPI project by DF [![Build Status](https://travis-ci.org/Texas-C/dfcc.svg?branch=master)](https://travis-ci.org/Texas-C/dfcc)

# Usage:



# Build:

## dependencies:

	glib gio2 soup2 xxhash 

Notice: `xxhash` may can not found in some of OS software source. It could be compile&install manual [xxhash](https://github.com/Cyan4973/xxHash)

install: __Debian command__ (example)

	sudo apt install git cmake make gcc libgio2.0-cil-dev libsoup2.4-cil-dev libsoup2.4-dev	libxxhash-dev

	make -j$(nproc)


# Generate code documents by `doxygen`

Install `doxygen` and `graphviz` (__Debian__):

	sudo apt install doxygen graphviz

Generate code documents:

	cd dfcc/
	doxygen Doxy.conf

And docs would output at `dfcc/docs/src_docs`. As `html` and `latex` format.

# TODO:

Write some docs?
