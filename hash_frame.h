// code by ntu ael

#ifndef HASH_FRAME_H
#define HASH_FRAME_H

struct struct_hashFrame { // 4 + 4 + 1 + 1 = 10 ian: updated on 20140207, after agreement by the team members
  unsigned long offset; // absolute first address
  unsigned long pages; // total number of pages ian: updated on 20140207, after agreement by the team members
  char stride; // each page stores stride number of frames TODO deprecated -> remove!
  char copies; // redundancy, typically 1 (=no redundancy) or 2 (=one backup copy)
};

extern struct struct_hashFrame g_hashFrame;

struct struct_frameIndex { // 4 + 1
  unsigned long page; //ian: updated on 20140207, following the update of the hashFrame.pages
  char index;
};

typedef struct struct_frameIndex FrameIndex;

char Hash_getFrame(FrameIndex* frameIndex, void* buffer, short length);
char Hash_putFrame(FrameIndex* frameIndex, void* buffer, short length);

#endif
