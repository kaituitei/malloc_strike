#define _GNU_SOURCE
# include <strings.h>
# include <dlfcn.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>

# define	MAX_MALLOC_CALLS	10000		// maximum number of malloc calls
# define	MAX_MALLOC_BYTES	1000000		// maximum number of bytes allocated (1MB default)
# define	MALLOC_ID_FAIL		-1UL		// forces the malloc of that ID to fail (-1UL to unset).
# define	PRINT_WARNINGS		true		// enable warnings (not necessary a fail)
# define	PRINT_CALLS			true		// print all malloc calls

typedef struct s_mtrace
{
    bool    malloc_pass;
    bool    malloc_lock;
    bool    free_lock;
    size_t  malloc_count;
	size_t  malloc_id;
    size_t  last_count;
    size_t  mem_used;
    size_t  malloc_size[MAX_MALLOC_CALLS];
    void    *malloc_lst[MAX_MALLOC_CALLS];
}   t_mtrace;

static t_mtrace mtrace_stack;

extern void    *malloc(size_t m)
{
    if (!mtrace_stack.malloc_pass \
        && (mtrace_stack.malloc_id + 1 == MALLOC_ID_FAIL || \
            mtrace_stack.malloc_count >= MAX_MALLOC_CALLS || \
            mtrace_stack.mem_used + m > MAX_MALLOC_BYTES)
    )
    {
        mtrace_stack.malloc_pass = true;
        mtrace_stack.malloc_lock = true;
        #if PRINT_CALLS
            dprintf(2, "\x1b[32m#%zu\x1b[36m Malloc(%zu): \x1b[31mArtificial malloc limit reached!\x1b[0m\n", mtrace_stack.malloc_id + 1, m);
        #endif
        dprintf(2, "\x1b[36mInfo:\x1b[0m Returning NULL to simulate allocation failure...\n");
        mtrace_stack.malloc_lock = false;
        mtrace_stack.malloc_pass = false;
        return NULL;
    }
    void *(*libc_malloc)(size_t) = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    void *p = libc_malloc(m);
    size_t i = mtrace_stack.last_count;
    if (mtrace_stack.malloc_lock)
        return p;
    mtrace_stack.malloc_lock = true;
    // malloc hook
    // find and fill a hole (de-fragmentation)
    while (i > 0 && mtrace_stack.malloc_lst[i] != NULL)
        i--;
    mtrace_stack.malloc_count++;
    if (mtrace_stack.malloc_count > mtrace_stack.last_count)
        mtrace_stack.last_count = mtrace_stack.malloc_count;
    mtrace_stack.malloc_lst[i] = p;
    mtrace_stack.malloc_size[i] = m;
    mtrace_stack.mem_used += m;
    mtrace_stack.malloc_id++;
    #if PRINT_CALLS
        dprintf(2, "\x1b[32m#%zu\x1b[36m Malloc(%zu): \x1b[33m%p\x1b[0m %zu bytes allocated.\n", mtrace_stack.malloc_id, m, p, mtrace_stack.mem_used);
    #endif
    mtrace_stack.malloc_lock = false;
    return p;
}

void    free(void *ptr)
{
    void *(*libc_free)(void *) = (void *(*)(void *))dlsym(RTLD_NEXT, "free");
    if (ptr == NULL)
    {
        #if PRINT_WARNINGS
            mtrace_stack.malloc_lock = true;
            dprintf(2, "\x1b[33mWarning:\x1b[0m Trying to free NULL pointer.\n");
            mtrace_stack.malloc_lock = false;
        #endif
        return ;
    }
    libc_free(ptr);
    if (mtrace_stack.free_lock)
        return ;
    for (size_t i = mtrace_stack.last_count; i > 0; --i)
    {
        size_t indx = i - 1;
        if (mtrace_stack.malloc_lst[indx] == ptr)
        {
            mtrace_stack.free_lock = true;
            mtrace_stack.malloc_lock = true;
            // free hook
            mtrace_stack.mem_used -= mtrace_stack.malloc_size[indx];
            #if PRINT_CALLS
                dprintf(2, "\x1b[36mFree(\x1b[33m%p\x1b[36m):\x1b[0m %zu bytes freed, %zu bytes left.\n", ptr, mtrace_stack.malloc_size[indx], mtrace_stack.mem_used);
            #endif
            mtrace_stack.malloc_lst[indx] = NULL;
            mtrace_stack.malloc_size[indx] = 0;
            // reduce search space when i is at the end of the array (de-fragmentation)
            if (indx == mtrace_stack.last_count - 1)
                while (indx > 0 && !mtrace_stack.malloc_lst[--indx])
                    mtrace_stack.last_count--;
            mtrace_stack.malloc_count--;
            if (!mtrace_stack.malloc_count)
                dprintf(2, "\x1b[32mAll heaps were freed! No leaks was possible.\n\x1b[0m");
            mtrace_stack.malloc_lock = false;
            mtrace_stack.free_lock = false;
            break ;
        }
    }
}

