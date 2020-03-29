# A MPI project by DF [![Build Status](https://travis-ci.org/Texas-C/dfcc.svg?branch=master)](https://travis-ci.org/Texas-C/dfcc)

# Usage:



# Build:

## dependencies:

	glib gio2 soup2 xxhash

install: __Debian/Ubuntu command__ (example)

	sudo apt install git cmake make gcc libgio2.0-cil-dev libsoup2.4-cil-dev libsoup2.4-dev	golang-github-oneofone-xxhash-dev

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
