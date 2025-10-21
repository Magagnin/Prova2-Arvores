#ifndef PATRICIA_H
#define PATRICIA_H

#include <stdio.h>

typedef struct PatriciaNode {
    char *edge;            // label da aresta (sequência de '0'/'1')
    int edgeLen;          // comprimento de edge
    char *nextHop;        // se não NULL, este nó representa um prefixo com next-hop
    struct PatriciaNode *zero; // filho quando próximo bit é 0
    struct PatriciaNode *one;  // filho quando próximo bit é 1
} PatriciaNode;

typedef struct {
    PatriciaNode *root;
} Patricia;

Patricia* criarPatricia();
void destruirPatricia(Patricia *t);
int inserirPrefixo(Patricia *t, const char *bits, int nbits, const char *nextHop);
const char* buscarLongestPrefixMatch(Patricia *t, const char *bits, int nbits);
void imprimePatricia(Patricia *t);

#endif
