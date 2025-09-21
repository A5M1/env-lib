very easy to use .env libary for c/c++
```c
#include "env.h"
 
int main() {
    env_load(".env");
    const char* val = env_get("MY_KEY");
    env_inject_all(1);
    env_free();
}
```
