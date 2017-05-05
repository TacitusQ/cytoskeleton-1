.PHONY: clean

ICC = icpc -msse2 -qopt-report1 -qopt-report-phase=vec -shared-intel -march=native -no-multibyte-chars
#GCC = g++ -msse2 -ffast-math -Wall -g -march=native 
GCC = g++ -msse2 -ffast-math -Wall -g -march=native 
CFLAGS = -O3 -mcmodel=medium
LDFLAGS = -I.
objs = cytoskel.o


rsame: $(objs)
	$(ICC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp 
	$(ICC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o *~ *.txt *.xyz
