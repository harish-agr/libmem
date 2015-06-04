# libmem

A collection of modules for managing memory in C/C++:

* `allocator_t` - a memory allocator abstraction with built-in default, aligned, counted,
  guarded, and traced allocators

* `buffer_t` - a growable memory buffer

* `pool_t` - a pool of fixed size, fixed address objects


## Building

The project is built using cmake - an out-of-source build is recommended, for example:

	mkdir build
	cd build
	cmake ../

If you are generating Makefiles with cmake (the default on Linux and Mac), then you can
run the following command to build the library and tests:

	make

This project is built with the following strict compiler flags by default:

* -std=c90
* -Wall
* -Wextra
* -Werror
* -Wmissing-prototypes
* -Wstrict-prototypes
* -Wold-style-definition
* -pedantic-errors

You can turn off this behaviour by disabling the `STRICT` option, e.g:

	cmake -DSTRICT=False ...


## Testing

Once you have built everything, you can run the tests with:

	make test

If valgrind is found on your machine, then the tests will also be run under valgrind
by default. You can disable this by setting the `VALGRIND_ENABLE` option, e.g:

	cmake -DVALGRIND_ENABLE=False ...


## Installing

To install into the default location for user libraries on your system, run:

	make install

If you wish to provide a different installation location, you can do so via
`CMAKE_INSTALL_PREFIX`, e.g:

	cmake -DCMAKE_INSTALL_PREFIX=your_path_goes_here ...
	make install


## Known Issues

* Calling `pool_return` on an object already returned to the pool will corrupt the pool

