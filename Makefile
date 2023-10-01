conn: conn.o
	gcc conn.o -lmenuw -lncurses -DNCURSES_STATIC -o conn.exe

conn.o: conn.c
	gcc -c conn.c -lmenuw -lncurses -DNCURSES_STATIC

clean:
	rm *.o conn.exe
