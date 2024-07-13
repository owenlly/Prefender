all: flush_reload evict_reload prime_probe

flush_reload:
	gcc flush_reload.c -o flush_reload.out

evict_reload:
	gcc evict_reload.c -o evict_reload.out

prime_probe:
	gcc prime_probe.c -o prime_probe.out

clean:
	rm -rf *.out