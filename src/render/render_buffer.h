#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

typedef enum RND_Buffer_Type {
  RND_BUFFER_VERTEX,
  RND_BUFFER_INDEX,
  RND_BUFFER_UNIFORM,
  RND_BUFFER_COUNT,
} RND_Buffer_Type;

typedef struct RND_Buffer RND_Buffer;
struct RND_Buffer {
  RND_Buffer_Type type;
};

#endif // RENDER_BUFFER_H
