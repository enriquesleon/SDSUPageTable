CC = gcc
DEPS = memory_page.h memory_trace_options.h byutr.h
CFLAGS =-c -Wall
OBJS = memory_page.o memory_trace_options.o byu_tracereader.o main.o

pagetable:$(OBJS)
	$(CC) $(OBJS) -o pagetable -lm
memory_page.o: memory_page.c memory_page.h
	$(CC) $(CFLAGS) memory_page.c
memory_trace_options.o: memory_trace_options.c memory_trace_options.h
	$(CC) $(CFLAGS) memory_trace_options.c
byu_tracreader.o: byu_tracereader.c byutr.h
	$(CC) $(CFLAGS) byu_tracereader.c
main.o: $(DEPS)
	$(CC) $(CFLAGS) main.c
clean:
	rm pagetable $(OBJS)