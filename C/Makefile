YEAR=2019

all: performance

gcc: competition$(YEAR).c
	$@ -DDEBUG -mtune=generic -O0 $^ -o a.out 

clang: competition$(YEAR).c
	$@ -DDEBUG -mtune=generic -O0 $^ -o a.out 

performance:
	/usr/bin/time -f'%E' ./a.out ; sleep 1 ;

clean:
	rm -f a.out *.txt ;
