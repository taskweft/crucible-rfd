#ifndef CRUCIBLE_NIF_H
#define CRUCIBLE_NIF_H

/*
 * Taskweft NIF — C ABI for the HTN planner.
 *
 * The taskweft_nif shared object is loaded at server startup via
 * dlopen/dlsym.  It exposes a stable C ABI:
 *
 *   taskweft_plan(domain_json, state_json, &plan_out)
 *     → 0 on success, -1 on error
 *     → plan_out must be freed by the caller via taskweft_free
 *
 *   taskweft_validate(domain_json)
 *     → 0 if the domain is valid, -1 on error
 *
 *   taskweft_free(ptr)
 *     → frees memory allocated by the NIF
 *
 * Build from: https://github.com/taskweft/taskweft
 * Drop the .so into src/nif/ before building the server.
 */

#include <stddef.h>

/* Function pointer types matching the taskweft_nif C ABI. */
typedef int  (*taskweft_plan_fn)(const char *domain_json,
                                  const char *state_json,
                                  char **plan_out);
typedef int  (*taskweft_validate_fn)(const char *domain_json);
typedef void (*taskweft_free_fn)(void *ptr);

/* Loaded handles — set during server init. */
typedef struct {
    void                *handle;    /* dlopen handle */
    taskweft_plan_fn     plan;
    taskweft_validate_fn validate;
    taskweft_free_fn     free;
} taskweft_nif_t;

/* Load the NIF from path. Returns 0 on success. */
int taskweft_nif_load(taskweft_nif_t *nif, const char *so_path);

/* Unload. */
void taskweft_nif_unload(taskweft_nif_t *nif);

#endif /* CRUCIBLE_NIF_H */
