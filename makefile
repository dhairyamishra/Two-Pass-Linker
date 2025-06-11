all:
	g++ -Wall -Wextra -O2 -o linker linker.cpp

test:
	mkdir -p final_outdir
	chmod +x runit.sh
	./runit.sh final_outdir ./linker

grade:
	chmod +x gradeit.sh
	./gradeit.sh . final_outdir

logs:
	(hostname; make clean; make 2>&1) > make.log
	./gradeit.sh . final_outdir > gradeit.log

runall: all test grade logs

clean:
	rm -f linker
	rm -rf final_outdir
	rm -f make.log gradeit.log lab1_submit.zip
