#ifndef CRUCIBLE_WORLD_OUTPUT_H
#define CRUCIBLE_WORLD_OUTPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * World output — structured narrative message emitted by the server
 * on each tick.  C struct equivalent of the original Elixir DSL.
 * Serialized via the encoder function table (world_output_serializer).
 */

typedef enum {
    WORLD_OUTPUT_SOURCE_WORLD  = 0,
    WORLD_OUTPUT_SOURCE_NPC    = 1,
    WORLD_OUTPUT_SOURCE_SYSTEM = 2,
    WORLD_OUTPUT_SOURCE_ERROR  = 3,
} world_output_source_t;

typedef enum {
    WORLD_OUTPUT_TAG_ROOM_DESC = 0,
    WORLD_OUTPUT_TAG_DIALOGUE  = 1,
    WORLD_OUTPUT_TAG_COMBAT    = 2,
    WORLD_OUTPUT_TAG_EVENT     = 3,
    WORLD_OUTPUT_TAG_SYSTEM    = 4,
} world_output_tag_t;

typedef struct {
    world_output_source_t  source;
    const char            *source_name;  /* NPC name, NULL for world/system/error */
    const char            *body;         /* Narrative text (UTF-8) */
    const world_output_tag_t *tags;      /* Tag array */
    size_t                 tag_count;
} world_output_t;

/* Constructor — copies strings (caller must free via world_output_destroy). */
world_output_t world_output_make(world_output_source_t source,
                                  const char *source_name,
                                  const char *body,
                                  const world_output_tag_t *tags,
                                  size_t tag_count);

void world_output_destroy(world_output_t *out);

/*
 * Serializer function table — decouples the struct from its
 * wire representation (same pattern as the original Elixir
 * Encoder protocol, but C function pointers).
 */
typedef struct {
    bool   (*encode_binary)(const world_output_t *out, uint8_t **buf, size_t *len);
    char  *(*encode_json)(const world_output_t *out);
} world_output_serializer_t;

extern const world_output_serializer_t world_output_serializer;

#endif /* CRUCIBLE_WORLD_OUTPUT_H */
