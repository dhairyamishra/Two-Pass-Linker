all:
	g++ -Wall -Wextra -O2 -o linker linker.cpp

test:
	mkdir -p outdir
	chmod +x runit.sh
	./runit.sh outdir ./linker

grade:
	chmod +x gradeit.sh
	./gradeit.sh ./sample_outputs outdir

runall: all test grade

clean:
	rm -f linker
	rm -rf outdir
