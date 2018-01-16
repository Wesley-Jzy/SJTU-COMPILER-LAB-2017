#ifndef ESCAPE_H
#define ESCAPE_H

typedef struct escapeEntry_ *escapeEntry;

struct escapeEntry_ {
    int depth;
    bool *escape;
};

void Esc_findEscape(A_exp exp);

#endif
