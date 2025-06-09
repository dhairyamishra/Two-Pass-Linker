all:
	g++ -Wall -Wextra -O2 -o linker linker.cpp

test:
	mkdir -p final_outdir
	chmod +x runit.sh
	./runit.sh final_outdir ./linker

grade:
	chmod +x gradeit.sh
	./gradeit.sh ./sample_outputs final_outdir

runall: all test grade

clean:
	rm -f linker
	rm -rf final_outdir
