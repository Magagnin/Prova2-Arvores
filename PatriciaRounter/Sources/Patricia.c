#include "Patricia.h"
#include <stdlib.h>
#include <string.h>

/* Utils internos */
static char* strndup_m(const char* s, int n){
    char* r = (char*) malloc(n+1);
    memcpy(r, s, n);
    r[n]=0;
    return r;
}

static PatriciaNode* novoNo(const char* edge, int edgeLen, const char* nextHop){
    PatriciaNode* p = (PatriciaNode*) malloc(sizeof(PatriciaNode));
    p->edge = edgeLen>0 ? strndup_m(edge, edgeLen) : NULL;
    p->edgeLen = edgeLen;
    p->nextHop = nextHop ? strdup(nextHop) : NULL;
    p->zero = p->one = NULL;
    return p;
}

static void liberaNo(PatriciaNode* n){
    if(!n) return;
    if(n->edge) free(n->edge);
    if(n->nextHop) free(n->nextHop);
    liberaNo(n->zero);
    liberaNo(n->one);
    free(n);
}

/* cria/destrói */
Patricia* criarPatricia(){
    Patricia* t = (Patricia*) malloc(sizeof(Patricia));
    t->root = novoNo(NULL, 0, NULL);
    return t;
}

void destruirPatricia(Patricia *t){
    if(!t) return;
    liberaNo(t->root);
    free(t);
}

/* retorna o número de bits iguais iniciais entre s1 e s2 */
static int commonPrefix(const char* s1, int n1, const char* s2, int n2){
    int i=0;
    while(i<n1 && i<n2 && s1[i]==s2[i]) i++;
    return i;
}

/*
 Inserção:
 - Mantemos rótulos por aresta (edge).
 - Percorremos consumindo rótulos; se divergirem, fazemos split.
 - Se chegarmos no fim da chave, atribuimos nextHop ao nó.
 Retorno: 1 sucesso, 0 se prefixo conflitante (ex: já existe prefixo igual)
*/
int inserirPrefixo(Patricia *t, const char *bits, int nbits, const char *nextHop){
    PatriciaNode *cur = t->root;
    int pos = 0; // posição em bits
    while(1){
        // se cur.edge consome parte da chave restante...
        if(cur->edgeLen > 0){
            int cp = commonPrefix(cur->edge, cur->edgeLen, bits+pos, nbits-pos);
            if(cp < cur->edgeLen){
                // split do nó cur: prefixo comum -> novo nó intermediário
                // cria nó filho que mantém o sufixo existente
                PatriciaNode* child = novoNo(cur->edge + cp, cur->edgeLen - cp, cur->nextHop);
                child->zero = cur->zero;
                child->one  = cur->one;

                // ajusta cur para conter a parte comum
                free(cur->edge);
                cur->edge = strndup_m(bits+pos, cp); // parte comum
                cur->edgeLen = cp;
                // cur deixa de ter nextHop (só o child mantém se existia)
                if(cur->nextHop){ free(cur->nextHop); cur->nextHop = NULL; }

                // reset children
                cur->zero = cur->one = NULL;
                // conectar child na direção correta
                if(child->edgeLen>0){
                    if(child->edge[0] == '0') cur->zero = child;
                    else cur->one = child;
                } else {
                    // shouldn't happen
                    if(child) { cur->zero = child; }
                }

                // agora inserir o restante da chave
                pos += cp;
                if(pos == nbits){
                    // a chave termina exatamente no nó cur -> coloca nextHop aqui
                    if(cur->nextHop) return 0; // já existia
                    cur->nextHop = strdup(nextHop);
                    return 1;
                } else {
                    // criar novo filho com o sufixo restante
                    int rem = nbits - pos;
                    PatriciaNode* novo = novoNo(bits+pos, rem, nextHop);
                    if(bits[pos]=='0') cur->zero = novo; else cur->one = novo;
                    return 1;
                }
            } else {
                // consumiu todo cur->edge
                pos += cp;
                if(pos > nbits) return 0; // impossível
                if(pos == nbits){
                    // chave consumida: set nextHop no nó cur
                    if(cur->nextHop) return 0; // já existe
                    cur->nextHop = strdup(nextHop);
                    return 1;
                } else {
                    // precisa seguir para filho apropriado
                    char nextBit = bits[pos];
                    PatriciaNode** nextPtr = (nextBit=='0') ? &cur->zero : &cur->one;
                    if(*nextPtr == NULL){
                        // cria novo filho com sufixo restante
                        int rem = nbits - pos;
                        PatriciaNode* novo = novoNo(bits+pos, rem, nextHop);
                        *nextPtr = novo;
                        return 1;
                    } else {
                        cur = *nextPtr;
                        continue;
                    }
                }
            }
        } else {
            // cur->edgeLen == 0 (root or splitted intermediate without label)
            if(pos == nbits){
                if(cur->nextHop) return 0;
                cur->nextHop = strdup(nextHop);
                return 1;
            }
            char nextBit = bits[pos];
            PatriciaNode** nextPtr = (nextBit=='0') ? &cur->zero : &cur->one;
            if(*nextPtr == NULL){
                int rem = nbits - pos;
                PatriciaNode* novo = novoNo(bits+pos, rem, nextHop);
                *nextPtr = novo;
                return 1;
            } else {
                cur = *nextPtr;
                continue;
            }
        }
    }
}

/* busca LPM: percorre consumindo arestas; guarda último nextHop encontrado */
const char* buscarLongestPrefixMatch(Patricia *t, const char *bits, int nbits){
    PatriciaNode *cur = t->root;
    int pos = 0;
    const char *lastHop = cur->nextHop; // root may have default route
    while(cur){
        if(cur->edgeLen > 0){
            // must match entire edge to proceed
            if(pos + cur->edgeLen > nbits) break; // can't match full edge
            if( strncmp(cur->edge, bits+pos, cur->edgeLen) != 0 ) break; // mismatch
            pos += cur->edgeLen;
            if(cur->nextHop) lastHop = cur->nextHop;
            if(pos == nbits) break;
            if(bits[pos]=='0') cur = cur->zero; else cur = cur->one;
        } else {
            if(cur->nextHop) lastHop = cur->nextHop;
            if(pos == nbits) break;
            if(bits[pos]=='0') cur = cur->zero; else cur = cur->one;
            pos++;
        }
    }
    return lastHop;
}

/* impressão simples (pré-ordem) */
static void print_rec(PatriciaNode* n, int depth){
    if(!n) return;
    for(int i=0;i<depth;i++) printf("  ");
    printf("- edge=\"");
    if(n->edge) printf("%s", n->edge);
    printf("\"");
    if(n->nextHop) printf(" (prefix -> %s)", n->nextHop);
    printf("\n");
    print_rec(n->zero, depth+1);
    print_rec(n->one, depth+1);
}

void imprimePatricia(Patricia *t){
    printf("PATRICIA:\n");
    print_rec(t->root, 0);
}


