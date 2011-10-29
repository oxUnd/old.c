CC=gcc -g
DIR=src
G_H=$(DIR)/global.h
LIBS=-lpthread
scanBanner: main.o buffer.o array.o data_array.o data_string.o data_count.o data_integer.o request.o multitask.o
	$(CC) -o scanBanner   main.o buffer.o array.o data_array.o data_string.o data_count.o data_integer.o request.o multitask.o $(LIBS)
main.o: $(DIR)/main.c $(G_H) $(DIR)/array.h
	$(CC) -c $(DIR)/main.c $(G_H) $(DIR)/array.h
buffer.o: $(DIR)/buffer.h $(DIR)/buffer.c
	$(CC) -c $(DIR)/buffer.c $(DIR)/buffer.h
array.o: $(DIR)/array.h $(DIR)/array.c
	$(CC) -c $(DIR)/array.h $(DIR)/array.c
data_array.o: $(DIR)/array.h $(DIR)/data_array.c
	$(CC) -c $(DIR)/array.h $(DIR)/data_array.c
data_string.o: $(DIR)/array.h $(DIR)/data_string.c
	$(CC) -c $(DIR)/data_string.c
data_count.o: $(DIR)/array.h $(DIR)/data_count.c
	$(CC) -c $(DIR)/array.h $(DIR)/data_count.c
data_integer.o: $(DIR)/array.h $(DIR)/data_integer.c
	$(CC) -c $(DIR)/array.h $(DIR)/data_integer.c
request.o: $(DIR)/array.h $(G_H) $(DIR)/request.h $(DIR)/request.c
	$(CC) -c $(DIR)/array.h $(G_H) $(DIR)/request.h $(DIR)/request.c
multitask.o: $(G_H) $(DIR)/multitask.h
	$(CC) -c $(DIR)/multitask.c
clean:
	rm *.o
