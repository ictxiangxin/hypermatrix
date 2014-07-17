test: test.cxx hypermatrix.h
	g++ -O3 -o test test.cxx
	python test.py > input.txt
	./test < input.txt
