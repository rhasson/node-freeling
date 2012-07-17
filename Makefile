all:
	cd deps/freeling && $(MAKE) static
	node-waf configure build

clean:
	cd deps/freeling && $(MAKE) clean
	rm -rf build

temp:
	rm -rf tmp/freeling
	mkdir -p tmp/freeling
	cp -r README *.{cc,h,js*} wscript Makefile deps tmp/freeling
	cd tmp/freeling && rm -rf deps/*/.git* deps/*/*.o

package: temp
	cd tmp && tar -czvf freeling.tgz freeling