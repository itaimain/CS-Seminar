#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* The function an attacker should NOT be able to reach. */
void win()
{
    printf("you win! control flow hijacked via heap overflow\n");
}

void boring()
{
    printf("nothing interesting happened\n");
}

/*
 * A heap-allocated object: a fixed-size buffer immediately followed by a
 * function pointer. This mirrors a common real-world pattern (buffer +
 * vtable/callback living in the same heap chunk).
 */
typedef struct
{
    char buffer[32];
    void (*callback)(void);
} heap_obj_t;

int main()
{
    heap_obj_t *obj = malloc(sizeof(heap_obj_t));
    obj->callback = boring;

    printf("enter data: ");
    fflush(stdout);

    /* Vulnerable: reads up to 128 bytes into a 32-byte buffer, no bound
     * check against sizeof(obj->buffer). A long enough input overflows into
     * the adjacent obj->callback field. */
    read(0, obj->buffer, 128);

    obj->callback();

    free(obj);
    return 0;
}
