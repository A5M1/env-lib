#include "..\env\env.h"
#include <stdio.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
int main(void) {
	if(env_load(".env")!=0) {
		fprintf(stderr, "failed to load .env\n"); return 1;
	}
	const char* a=env_get("API_KEY");
	const char* b=env_get("USER");
	printf("library API_KEY=%s\n", a ? a : "(not found)");
	printf("library USER=%s\n", b ? b : "(not found)");
	if(env_inject_all(1)!=0) {
		fprintf(stderr, "failed to inject env\n"); env_free(); return 1;
	}
	const char* ge=getenv("API_KEY");
	printf("process getenv API_KEY=%s\n", ge ? ge : "(not found)");
	env_free();
	return 0;
}
