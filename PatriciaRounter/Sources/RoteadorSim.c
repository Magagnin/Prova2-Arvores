#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Patricia.h"   // cabeçalho da árvore PATRICIA

/* =============================== */
/* Funções auxiliares para IPv4    */
/* =============================== */

// Converte um IP "a.b.c.d" em uma string de 32 bits '0'/'1'
static void ipv4_to_bits(const char* ip, char *outbits) {
    unsigned a, b, c, d;
    sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
    unsigned long v = (a << 24) | (b << 16) | (c << 8) | d;
    for (int i = 0; i < 32; i++) {
        int bit = (v & (1u << (31 - i))) ? 1 : 0;
        outbits[i] = bit ? '1' : '0';
    }
    outbits[32] = '\0';
}

// Converte "192.168.0.0/16" → bits e comprimento (len)
static void parse_prefix(const char* prefix, char *outbits, int *outlen) {
    char ip[32];
    int plen;
    sscanf(prefix, "%31[^/]/%d", ip, &plen);
    char tmp[33];
    tmp[32] = '\0';
    ipv4_to_bits(ip, tmp);
    memcpy(outbits, tmp, plen);
    *outlen = plen;
}

/* =============================== */
/* Estrutura de roteador           */
/* =============================== */

typedef struct Router {
    char name[32];
    Patricia *table;
} Router;

// Procura um roteador pelo nome
static Router* findRouter(Router *routers, int n, const char* name) {
    for (int i = 0; i < n; i++)
        if (strcmp(routers[i].name, name) == 0)
            return &routers[i];
    return NULL;
}

/* =============================== */
/* Função de simulação de rotas    */
/* =============================== */

void route_packet(Router *routers, int nrouters, const char* srcRouterName, const char* dstIP) {
    Router* cur = findRouter(routers, nrouters, srcRouterName);
    if (!cur) {
        printf("Router %s não encontrado\n", srcRouterName);
        return;
    }

    char bits[33];
    bits[32] = 0;
    ipv4_to_bits(dstIP, bits);

    int maxHops = 50;
    printf("\nRoteando pacote para %s a partir de %s\n", dstIP, srcRouterName);
    while (maxHops--) {
        const char* nh = buscarLongestPrefixMatch(cur->table, bits, 32);
        printf("  Router %s: next-hop = %s\n", cur->name, nh ? nh : "NULL");

        if (nh == NULL) {
            printf("  Sem rota em %s -> pacote descartado\n", cur->name);
            return;
        }

        if (strcmp(nh, "LOCAL") == 0) {
            printf("  Pacote entregue em %s (next-hop LOCAL)\n", cur->name);
            return;
        }

        Router* next = findRouter(routers, nrouters, nh);
        if (!next) {
            printf("  Próximo salto %s não encontrado -> pacote descartado\n", nh);
            return;
        }

        if (next == cur) {
            printf("  Loop detectado!\n");
            return;
        }

        cur = next;
    }

    printf("  Excedeu limite de hops -> loop detectado\n");
}

/* =============================== */
/* Função principal (main)         */
/* =============================== */

int main() {
    Router routers[4];

    // variáveis auxiliares para manipular prefixos
    char bits[33];
    int bl;

    // Inicializa roteadores
    strcpy(routers[0].name, "A");
    routers[0].table = criarPatricia();

    strcpy(routers[1].name, "B");
    routers[1].table = criarPatricia();

    strcpy(routers[2].name, "C");
    routers[2].table = criarPatricia();

    strcpy(routers[3].name, "D");
    routers[3].table = criarPatricia();

    /* ---------------------- */
    /* Tabelas de roteamento  */
    /* ---------------------- */

    // === Router A ===
    parse_prefix("10.0.0.0/8", bits, &bl);
    inserirPrefixo(routers[0].table, bits, bl, "B");

    parse_prefix("192.168.0.0/16", bits, &bl);
    inserirPrefixo(routers[0].table, bits, bl, "C");

    parse_prefix("0.0.0.0/0", bits, &bl);
    inserirPrefixo(routers[0].table, bits, bl, "B"); // rota padrão


    // === Router B ===
    parse_prefix("192.168.1.0/24", bits, &bl);
    inserirPrefixo(routers[1].table, bits, bl, "LOCAL");

    parse_prefix("10.0.0.0/8", bits, &bl);
    inserirPrefixo(routers[1].table, bits, bl, "C");

    // rota padrão
    parse_prefix("0.0.0.0/0", bits, &bl);
    inserirPrefixo(routers[1].table, bits, bl, "C");


    // === Router C ===
    parse_prefix("10.0.0.0/8", bits, &bl);
    inserirPrefixo(routers[2].table, bits, bl, "D");

    parse_prefix("172.16.0.0/12", bits, &bl);
    inserirPrefixo(routers[2].table, bits, bl, "LOCAL");

    // rota padrão
    parse_prefix("0.0.0.0/0", bits, &bl);
    inserirPrefixo(routers[2].table, bits, bl, "D");


    // === Router D ===
    parse_prefix("10.0.0.0/8", bits, &bl);
    inserirPrefixo(routers[3].table, bits, bl, "LOCAL");

    /* ---------------------- */
    /* Impressão das tabelas  */
    /* ---------------------- */
    printf("Tabela A:\n");
    imprimePatricia(routers[0].table);
    printf("\nTabela B:\n");
    imprimePatricia(routers[1].table);
    printf("\nTabela C:\n");
    imprimePatricia(routers[2].table);
    printf("\nTabela D:\n");
    imprimePatricia(routers[3].table);

    /* ---------------------- */
    /* Simulações             */
    /* ---------------------- */
    printf("\n--- Simulações ---\n");
    route_packet(routers, 4, "A", "192.168.1.5"); // A → B → LOCAL
    route_packet(routers, 4, "A", "10.1.2.3");    // A → B → C → D → LOCAL
    route_packet(routers, 4, "A", "172.16.5.4");  // A → B → C → LOCAL

    /* ---------------------- */
    /* Libera memória         */
    /* ---------------------- */
    for (int i = 0; i < 4; i++)
        destruirPatricia(routers[i].table);

    return 0;
}
