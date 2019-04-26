shunt: $(wildcard src/*.c)
	gcc -o $@ $^

default: shunt

clean:
	rm -rf ./shunt
