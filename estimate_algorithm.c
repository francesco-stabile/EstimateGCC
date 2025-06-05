#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <sys/resource.h>
#include <graphviz/cgraph.h>

#define FIFO true
#define LIFO false

/**
 * Da lec2.pdf pag. 3/11
 * 1. Let k:= 2 /ε² ·ln (2/δ).
 * 2. For i= 1 to k do the following:
 * • Sample a vertex vi uniformly at random from v (with replacement).
 * • For this vi compute Xi := 1 / s′vi by doing a DFS/BFS from vi and truncating the search once 2 /ε
 * vertices are visited.
 * 3. Output Ċ = n/k·∑i=1 to k Xi.
 * 
 * Argomenti:
 *   argv[1] = epsilon (double)
 *   argv[2] = delta   (double)
 *   Opzionale [argv[3] = input .dot file]
 */

/*
Funzioni della libreria cgraph.h (cgraph.pdf)
    Pagina 4/31 (Graph)
    • g = agread(stdin,NULL);
    Here, the graph’s name, type and contents including attributes depend on the file contents. 
    (The second argument is the same optional method pointer mentioned above for agopen).
    • agnnodes(g), agnedges(g) and agnsubg(g) return the count of nodes, edges and (immediate) subgraphs in a graph (or subgraph).
    To delete a graph and its associated data structures, freeing their memory, one uses:
    • agclose(g);
    You can write a representation of a graph to a file:
    • g = agwrite(g,stdout);
    agwrite creates an external representation of a graph’s contents and attributes

    Pagina 5/31 (Nodes)
    In Cgraph, a node is usually identified by a unique string name and a unique internal integer ID assigned by
    Cgraph. (For convenience, you can also create ”anonymous” nodes by giving NULL as the node name.) A node
    also has in- and out-edge sets, even in undirected graphs.
    Once you have a graph, you can create or search for nodes this way:
    Agnode_t *n;
    • n = agnode(g,"node28",TRUE);
    The first argument is a graph or subgraph in which the node is to be created. The second is the name
    (or NULL for anonymous nodes.) When the third argument is TRUE, the node is created if it doesn’t already
    exist. When it’s FALSE, as shown below, then Cgraph searches to locate an existing node with the given name,
    returning NULL if none is found.
    • n = agnode(g,"node28",FALSE);

    Pagina 6/31 (Edges e Traversal)
    Cgraph has functions for iterating over graph objects. For example, we can scan all the edges of a graph (directed
    or undirected) by the following:
    • for (n = agfstnode(g); n; n = agnxtnode(g,n)) {
        for (e = agfstout(g,n); e; e = agnxtout(g,e)) {
            //do something
        }
    }
    •agtail(e) and aghead(e) return the endpoints of e (aghead è la destinazione, quindi se creo un arco tra v e u, u sarà la destinazione (head) mentre v sarà l'origine (tail))

    Pagina 7/31 (Traversal)
    To visit all the edges of a node in an undirected graph:
    • for (e = agfstedge(g,n); e; e = agnxtedge(g,e,n))
        // do something
    Be careful if your code deletes an edge or node during the traversal, as then the object will no longer be
    valid to get the next object. This is typically handled by code like:
    • for (e = agfstedge(g,n); e; e = f) {
        f = agnxtedge(g,e,n))
        //delete e 
        }
    Traversals are guaranteed to visit the nodes of a graph, or edges of a node, in their order of creation in the root graph
    • agnxtedge(g, e, n): Restituisce il prossimo arco collegato al nodo n dopo e
    • agfstedge(g, n): Restituisce il primo arco collegato al nodo n nel grafo g

*/

/// @brief QueueNode serve per modellare un nodo all'interno della coda. Ogni nodo (Agnode_t* node) punta a quello successivo con il puntatore next (QueueNode*)
typedef struct QueueNode {
    Agnode_t* node;
    struct QueueNode* next;
} QueueNode;

/// @brief Queue serve per modellare la vera e propria coda con front e rear, sono due puntatori alla struttura QueueNode, type: FIFO o LIFO
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    bool type;
} Queue;

/// @brief inizializza la coda q
/// @param q la coda da inizializzare
/// @param type FIFO per BFS , LIFO per DFS
void init_queue(Queue* q, bool type){
    q->front = NULL;
    q->rear = NULL;
    q->type = type;
}

/// @brief Aggiunge il nodo node alla coda q, in coda se è FIFO, in testa se è LIFO
/// @param q la coda in cui aggiungere il nodo node
/// @param node il nodo da aggiungere nella coda q
void enqueue(Queue* q, Agnode_t* node) {
    QueueNode* newNode = malloc(sizeof(QueueNode));
    newNode->node = node;
    newNode->next = NULL;

    if(q->type == FIFO){
        if(q->rear == NULL){
            q->front = q->rear = newNode;
        }
        else{
            q->rear->next = newNode;
            q->rear = newNode;
        }
    }
    else{
        if(q->rear == NULL){
            q->front = q->rear = newNode;
        }
        else{
            newNode->next = q->front;
            q->front = newNode;
        }
    }
}

/// @brief Rimuove dalla testa della coda q il nodo e lo restituisce
/// @param q la coda da cui rimuovere il nodo
/// @return il nodo rimosso (NULL se è vuota)
Agnode_t* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    
    QueueNode* temp = q->front;
    Agnode_t* node = temp->node;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    return node;
}

/// @brief Controlla se la coda è vuota, restituisce true (1) se la coda q è vuota, 0 (false) altrimenti
/// @param q la coda da controllare
/// @return true (1) se la coda q è vuota, 0 (false) altrimenti
bool isEmpty(Queue* q) {
    return q->front == NULL;
}

/// @brief Restituisce l'indice del nodo target presente nell'index map, -1 se non è presente
/// @param index_map l'array che contiene i nodi
/// @param total_nodes il numero totale dei nodi nel grafo, quindi la dimensione di index_map
/// @param target il nodo di cui si sta cercando l'indice
/// @return l'indice del nodo target all'interno di index map se esiste, -1 altrimenti
int find_index(Agnode_t **index_map, long total_nodes, Agnode_t *target) {
    for (long i = 0; i < total_nodes; i++) {
        if (index_map[i] == target)
            return i;
    }
    return -1;
}


/// @brief Esegue una ricerca BFS/DFS sul grafo g a partire dal nodo start e si ferma se raggiunge limit nodi oppure se la coda è vuota.
/// @param g il grafo da cui eseguire la ricerca
/// @param start il nodo da cui iniziare la ricerca
/// @param limit il numero massimo di nodi da esplorare
/// @param type FIFO per ricerca BFS, LIFO per ricerca DFS
/// @return il numero di nodi visitati
long Search_truncated(Agraph_t* g, Agnode_t* start, long limit, bool type){
    Queue q;
    init_queue(&q, type);
    enqueue(&q, start);
    long count = 1;
    long total_nodes = (long) agnnodes(g);
    Agnode_t **index_map = malloc(sizeof(Agnode_t*) * total_nodes);
    bool *visited = calloc(total_nodes, sizeof(bool));

    long i = 0;
    for (Agnode_t *v = agfstnode(g); v; v = agnxtnode(g, v)) {
        index_map[i] = v;
        i++;
    }
    int start_index = find_index(index_map, total_nodes, start);
    visited[start_index] = true;
    
    while (!isEmpty(&q) && count < limit)
    {
        Agnode_t* current = dequeue(&q);
        for (Agedge_t* e = agfstedge(g, current); e; e = agnxtedge(g,e,current))
        {
            if(count >= limit) break;
            /*
            Se l'origine dell'arco agtail(e) è proprio il nodo corrente, allora prendiamo il destinatario aghead(e), altrimenti prendiamo quello di origine agtail(e)
            */
            Agnode_t* neighbor = (agtail(e) == current) ? aghead(e) : agtail(e);
            int neighbor_index = find_index(index_map, total_nodes, neighbor);
            if(neighbor_index >= 0 && !visited[neighbor_index]){
                visited[neighbor_index] = true;
                count++;
                enqueue(&q, neighbor);
            }
        }
    }

    free(index_map);
    free(visited);
    while (!isEmpty(&q))
    {
        dequeue(&q);
    }
    
    return count;
}

/// @brief Stanpa la memoria utilizzata come maximum resident set size e il tempo di CPU trascorso
void print_stats(void) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    // Stampa memoria utilizzata
    printf("Memoria massima utilizzata: %.2f KB\n", usage.ru_maxrss / 1024.0);
    
    // Stampa tempo di esecuzione (user + system time)
    double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
    double system_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0;
    printf("Tempo di esecuzione (user): %.4f secondi\n", user_time);
    printf("Tempo di esecuzione (system): %.4f secondi\n", system_time);
    printf("Tempo di esecuzione totale: %.4f secondi\n", user_time + system_time);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Utilizzo: %s <epsilon> <delta> <input.dot> [BFS/DFS]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* 1. Converto epsilon e delta in double */
    double epsilon = atof(argv[1]);
    double delta   = atof(argv[2]);
    
    /* Verifico che i parametri siano tra 0 e 1 e che il file sia .dot*/
    if (epsilon <= 0 || epsilon >= 1) {
        fprintf(stderr, "Errore: epsilon deve essere compreso tra (0,1).\n");
        return EXIT_FAILURE;
    }
    if (delta <= 0 || delta >= 1) {
        fprintf(stderr, "Errore: delta deve essere compreso tra (0,1).\n");
        return EXIT_FAILURE;
    }
    /*Verifico che il file in input abbia l'estensione .dot*/
    const char *dot = strrchr(argv[3], '.');
    if(dot == NULL || dot == argv[3] || strcasecmp(dot, ".dot") != 0){
        fprintf(stderr, "Errore: il file fornito deve essere un file con estensione dot.\n");
        return EXIT_FAILURE;
    }

    bool type = FIFO;
    if(argc == 5 && !strcasecmp(argv[4], "DFS")){
        printf("Algoritmo con ricerca DFS\n");
        type = LIFO;
    }
    else{
        printf("Algoritmo con ricerca BFS (default)\n");
    }
    
    /* 2. Leggo il grafo dal file fornito .dot */
    Agraph_t *G = NULL;
    FILE *fp = fopen(argv[3], "r");
    if (!fp) {
        perror("Errore lettura file\n");
        return EXIT_FAILURE;
    }
    G = agread(fp, NULL);  
    fclose(fp);
    
    /* Ci assicuriamo che il grafo sia stato inizializzato correttamente*/
    if (!G) {
        fprintf(stderr, "Errore nel caricare o nel generare il grafo.\n");
        return EXIT_FAILURE;
    }
    /* 3. Passo 1: Calcoliamo k e lo arrotondiamo all'intero successivo con ceil
    k = ceil( (2/(epsilon*epsilon)) * ln(2/delta) ) */
    double k_double = (2.0 / (epsilon * epsilon)) * log(2.0 / delta);
    long k = (long)ceil(k_double);

    /* 4. Passo 2: Ciclo di campionamento: prendiamo k vertici casuali (con rimpiazzo)
    eseguiamo su di essi BFS/DFS troncato a 2/ε vertici massimi*/
    long n = agnnodes(G);
    if(n == 0){
        printf("Grafo vuoto, numero componenti: 0\n");
        return EXIT_SUCCESS;
    }
    srand((unsigned)time(NULL));

    /*Variabile per salvare ∑i=1 a k (Xi)*/
    double sum_X = 0.0;
    
    /* Per ogni campione */
    for (long i = 0; i < k; i++) {
        /* Scegliamo un indice casuale 0..n-1 */
        long r = rand() % n;
        /* Troviamo il nodo r-esimo tramite le funzioni di attraversamento (pagina 6/31) */
        Agnode_t *v = NULL;
        long idx = 0;
        for (v = agfstnode(G); v; v = agnxtnode(G, v)) {
            if (idx == r) break;
            idx++;
        }
        /* Eseguiamo BFS/DFS a partire da v, tronchiamo dopo limit = ceil(2/epsilon) nodi */
        long limit = (long)ceil(2.0 / epsilon);

        /* TODO: implementare BFS/DFS troncato e calcolare s'_v = dimensione della componente raggiungibile */
        long s_prime = Search_truncated(G, v, limit, type);
        
        /* Xi = 1.0 / s'_v */
        if (s_prime > 0) sum_X += 1.0 / s_prime;
    }

    /* 5. Fornisco la stima C_hat = n/k * sum_X */
    double C_hat = ((double)n / (double)k) * sum_X;
    printf("Numero di componenti stimate: %.6f\n", C_hat);

    /* 6. Pulizia */
    agclose(G);
    print_stats();
    return EXIT_SUCCESS;
}
