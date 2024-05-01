#include <stdio.h>
#include <string.h>

extern int
create_and_write_shared_memory(char* mmap_key,
	char* value,
	unsigned int size);

int
main(int argc, char* argv[]) {

	char* key = "/introduction";
	char* intro = "Hello, I'm Hizenberg.";
	create_and_write_shared_memory(key, intro, strlen(intro));
	return 0;
}