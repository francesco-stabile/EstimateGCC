# Algoritmo per la stima del numero di componenti connesse in un grafo

L'algoritmo affronta il problema della stima del numero di componenti connesse in un grafo.  
L'obiettivo è avere un algoritmo che opera in tempo sublineare.

Vogliamo stimare il numero di componenti connesse $C$ con due parametri $0 < \epsilon < 1$ e $0 < \delta < 1$  
tale che $P(|\widetilde{C} - C| \leq \epsilon n ) \geq 1 - \delta$ dove $C$ è il vero numero di componenti connesse del grafo $G$.

La strategia è ridurre il calcolo a una somma: per ogni vertice $v$ nel grafo, sia $s_{v}$ la dimensione della sua componente connessa, allora:

$$
C = \sum_{v \in V} \frac{1}{s_{v}}
$$

Poiché calcolare $s_{v}$ è particolarmente costoso se questo è grande, l'idea è stimare la somma di cui sopra campionando un sottoinsieme di vertici $V'$.

Definiamo $s_{v'} = \min(s_{v}, \frac{2}{\epsilon})$ per tutti i $v \in V'$ e $C' = \sum_{v \in V} \frac{1}{s_{v'}}$, allora:

$$
|C - C'| \leq \frac{\epsilon}{2} \cdot n
$$

In pratica calcoliamo $s_{v'}$ ottenendo comunque una buona stima di $C$. Il calcolo di $s_{v'}$ avviene facendo una ricerca BFS/DFS, iniziando dal vertice $v$ e terminando non appena più di $\frac{2}{\epsilon}$ vertici vengono visitati.

## Pseudocodice dell'algoritmo

1. $k = \frac{2}{\epsilon^2} \ln(\frac{2}{\delta})$
2. For $i = 1$ to $k$:
   - Campioniamo un vertice $v_{i}$ in maniera uniforme con rimpiazzamento
   - Calcoliamo $X_{i} = \frac{1}{s'_{v_{i}}}$ tramite BFS/DFS da $v_{i}$, troncando dopo aver visitato al massimo $\frac{2}{\epsilon}$ vertici
3. Restituiamo $\widetilde{C} = \frac{n}{k} \cdot \sum_{i=1}^k X_{i}$

Con questo algoritmo abbiamo:
- $\mathbb{E}[\widetilde{C}] = C'$
- $P(|\widetilde{C} - C'| \leq \frac{\epsilon}{2} \cdot n) \geq 1 - \delta$

Sfruttando la disuguaglianza triangolare otteniamo con probabilità $1 - \delta$:

$$
|\widetilde{C} - C| \leq |\widetilde{C} - C'| + |C' - C| \leq \frac{\epsilon}{2} \cdot n + \frac{\epsilon}{2} \cdot n = \epsilon \cdot n
$$

Con una complessità al runtime pari a $O(\frac{1}{\epsilon^{4}} \cdot \ln(\frac{1}{\delta}))$.

## Caratteristiche del programma

Il programma offre due strategie di ricerca:

- **BFS (Breadth-First Search)**: Implementata con coda FIFO
- **DFS (Depth-First Search)**: Implementata con pila LIFO

Prende in ingresso, da linea di comando, 3 parametri obbligatori ed un parametro opzionale.  
Ecco la sintassi del programma:

```bash
./estimate_algorithm <ε> <δ> <input.dot> [BFS|DFS]
```
dove
- **ε∈(0,1)**, indica l'errore di approssimazione.
- **δ∈(0,1)**, la probabilità di fallimento.
- **input.dot**, il file contente il grafo.
- **[BFS/DFS]**, il tipo di ricerca da eseguire, se non specificato, l'algoritmo verrà eseguito con ricerca BFS.

## Prerequisiti e installazione
L'algoritmo utilizza la libreria **cgraph** di Graphviz. Per installarla:
```bash
brew install graphviz
```
Assicurarsi che il Makefile includa i percorsi corretti per la libreria cgraph.

## Struttura della directory
La directory contiene:
- automated_test.sh: [vedi utilizzo qui](#compilazione-e-test)
- cgraph.pdf: tutorial della libreria cgraph reperibile da [qui](https://graphviz.org/pdf/cgraph.pdf)
- estimate_algorithm.c: il file sorgente dell'algoritmo
- grafo_enorme.dot: file di input per effettuare i test più onerosi.
- grafo.dot: file di input per effettuare i primi test.
- Makefile: file per la compilazione automatica [vedi utilizzo qui](#compilazione-e-test)

## Implementazione delle strutture dati

Per gestire le due strutture dati (FIFO e LIFO) ho deciso di definire due struct, una struct QueueNode che contine il nodo e quello successivo, mentre la struct Queue che mi rappresenta la struttura vera e propria, contiene un oggetto QueueNode che punta 'davanti' (front) e uno che punta 'dietro' (rear), inoltre contiene un parametro booleano (type) che può essere FIFO o LIFO, da questo parametro si decide se la struttura sarà rispettivamente una coda oppure una pila. L'idea è quella di utilizzare la funzione dequeue per estrarre sempre dalla testa, mentre enqueue a seconda del parametro type definito nella init_queue, andrà a inserire in coda se è stato specificato FIFO, in testa se è stato specificato LIFO.

```c
typedef struct QueueNode {
    Agnode_t* node;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    bool type;
} Queue;

void init_queue(Queue* q, bool type){
    q->front = NULL;
    q->rear = NULL;
    q->type = type;
}

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
```
## Gestione dei nodi del grafo
Poiché la libreria cgraph non consente la diretta indicizzazione dei nodi, ho deciso di creare una mappatura che mi consenta di indicizzare tutti i nodi del grafo, in questo modo posso effettuare i confronti necessari, ad esempio per vedere se un nodo è già stato visitato.

```c
long total_nodes = (long) agnnodes(g);
    Agnode_t **index_map = malloc(sizeof(Agnode_t*) * total_nodes);
    bool *visited = calloc(total_nodes, sizeof(bool));

    long i = 0;
    for (Agnode_t *v = agfstnode(g); v; v = agnxtnode(g, v)) {
        index_map[i] = v;
        i++;
    }

La funzione find_index mi permette di trovare, dato un nodo, l'indice di quest'ultimo.
int find_index(Agnode_t **index_map, long total_nodes, Agnode_t *target) {
    for (long i = 0; i < total_nodes; i++) {
        if (index_map[i] == target)
            return i;
    }
    return -1;
}
```

## Compilazione e test
Ho creato inoltre un Makefile per compilare in maniera efficiente, eseguire all'interno della directory il comando:
```bash
make
```

Inoltre ho creato un piccolo script in bash che mi consente di effettuare 4 test sull'algoritmo:
- File inesistente
- File non DOT
- Grafo vuoto
- Grafo valido
Per lanciare lo script eseguire il seguente comando all'interno della directory:
```bash
./automated_test.sh 
```
## Documentazione delle funzioni cgraph utilizzate
Di seguito riporto tutte le funzioni utilizzate della libreria cgraph, con il prototipo e il funzionamento di queste, il file cgraph.pdf contiene il tutorial con esempi pratici delle funzioni in questione, le informazioni sui prototipi sono reperibili [qui](https://graphviz.org/pdf/cgraph.3.pdf) e riassunte, la dicitura 'Equivalente a: <funzione>' indica come io l'ho utilizzata nel programma.

```c
/**
 * @brief Legge un grafo da un file in formato DOT
 * @param fp Puntatore al file da leggere
 * @param disc Puntatore al descrittore del grafo (NULL per grafi semplici)
 * @return Puntatore al grafo letto, NULL in caso di errore
 * Agraph_t *agread(FILE *fp, Agdisc_t *disc); 
 *
 * Equivalente a: G = agread(fp, NULL);
 * 
 * La funzione legge il grafo dal file fornito. Il nome del grafo, il tipo 
 * (directed/undirected) e il contenuto (inclusi gli attributi) dipendono
 * dal contenuto del file.
 */

/**
 * @brief Restituisce il numero di nodi in un grafo o sottografo
 * @param g Puntatore al grafo
 * @return Numero di nodi nel grafo
 * int agnnodes(Agraph_t *g);
 *
 * Equivalente a: agnnodes(g)
 */

/**
 * @brief Chiude un grafo liberando la memoria associata
 * @param g Puntatore al grafo da chiudere
 * void agclose(Agraph_t *g); 
 *
 * Equivalente a: agclose(G)
 * 
 * La funzione elimina il grafo e le strutture dati associate, liberando la memoria.
 */

/**
 * @brief Restituisce il primo nodo in un grafo
 * @param g Puntatore al grafo
 * @return Puntatore al primo nodo, NULL se il grafo è vuoto
 * Agnode_t *agfstnode(Agraph_t *g);
 * 
 * Equivalente a: agfstnode(G)
 * 
 * I nodi vengono restituiti nell'ordine di creazione nel grafo radice.
 */

/**
 * @brief Restituisce il nodo successivo in un grafo
 * @param g Puntatore al grafo
 * @param n Puntatore al nodo corrente
 * @return Puntatore al nodo successivo, NULL se non ce ne sono più
 * Agnode_t *agnxtnode(Agraph_t *g, Agnode_t *n); 
 *
 * Equivalente a: agnxtnode(g, v)
 */

/**
 * @brief Restituisce il primo arco incidente a un nodo (per grafi non diretti)
 * @param g Puntatore al grafo
 * @param n Puntatore al nodo
 * @return Puntatore al primo arco incidente, NULL se non ce ne sono
 * Agedge_t *agfstedge(Agraph_t *g, Agnode_t *n); 
 *
 * Equivalente a: agfstedge(g, current)
 */

/**
 * @brief Restituisce il prossimo arco incidente a un nodo (per grafi non diretti)
 * @param g Puntatore al grafo
 * @param e Puntatore all'arco corrente
 * @param n Puntatore al nodo
 * @return Puntatore al prossimo arco incidente, NULL se non ce ne sono più
 * Agedge_t *agnxtedge(Agraph_t *g, Agedge_t *e, Agnode_t *n); 
 *
 * Equivalente a: agnxtedge(g,e,current)
 */

/**
 * @brief Restituisce il nodo di origine di un arco
 * @param e Puntatore all'arco
 * @return Puntatore al nodo di origine (tail)
 * Agnode_t *agtail(Agedge_t *e) 
 *
 * Equivalente a: agtail(e)
 */

/**
 * @brief Restituisce il nodo di destinazione di un arco
 * @param e Puntatore all'arco
 * @return Puntatore al nodo di destinazione (head)
 * Agnode_t *aghead(Agedge_t *e); 
 *
 * Equivalente a: aghead(e)
 */
```
