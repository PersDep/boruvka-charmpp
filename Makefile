CHARMDIR = /home/kosarev/charm/netlrts-linux-x86_64/
CHARMC = $(CHARMDIR)/bin/charmc $(OPTS)


default: all
all: mst


mst : main.o mst.o
	$(CHARMC) -language charm++ -o mst main.o mst.o

main.o : main.C main.h main.decl.h main.def.h mst.decl.h
	$(CHARMC) -o main.o main.C

main.decl.h main.def.h : main.ci
	$(CHARMC) main.ci

mst.o : mst.C mst.h mst.decl.h mst.def.h main.decl.h
	$(CHARMC) -o mst.o mst.C

mst.decl.h mst.def.h : mst.ci
	$(CHARMC) mst.ci


clean:
	rm -f main.decl.h main.def.h main.o
	rm -f mst.decl.h mst.def.h mst.o
	rm -f mst charmrun
