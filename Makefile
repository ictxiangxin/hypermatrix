test: test.cxx hm.h input.txt
	g++ -O3 -DDEBUG -o test test.cxx
	./test < input.txt

input.txt:
	python test.py > input.txt

clean:
	rm -r test input.txt
