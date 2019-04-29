bin/%: src/%.c
	gcc -o $@ $<

clean:
	rm -rf ./bin/**
