test: test.cxx hypermatrix.h
	g++ -O3 -DDEBUG -o test test.cxx
	python test.py > input.txt
	./test < input.txt

clean:
	rm -r test input.txt
