#include "BST.h"
#include "PointDct.h"
#include "Point.h"  
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

// pour pouvoir l'utiliser dans pdctBallsearch
//devra être static 
typedef struct BNode_t {
    struct BNode_t *parent;
    struct BNode_t *left;
    struct BNode_t *right;
    void *key;
    void *value;
} BNode;

// Définir la structure de l'arbre comme elle est dans BST.c
typedef struct BST_t {
    BNode *root;
    size_t size;
    int (*compfn)(void *, void *);
} BST;

typedef struct {
    uint64_t key;
    void *value;
} Pair;

static uint64_t interleave8(uint8_t m, uint8_t n) {
    return (
        ((m * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 49) & 0x5555
    ) | (
        ((n * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 48) & 0xAAAA
    );
}

static uint64_t zEncode(uint32_t x, uint32_t y) {
    uint64_t out = 0;
    for (uint8_t byte = 0; byte < 4; ++byte) {
        out |= interleave8((x >> (byte * 8)) & 0xFF, (y >> (byte * 8)) & 0xFF) << (byte * 16);
    }
    return out;
}

static int comparePair(const void *a, const void *b);
static int compareUint64(void *a, void *b);

// Fonctions de comparaison 
static int compare_pair(const void *a, const void *b) {
    Pair *pa = (Pair*)a;
    Pair *pb = (Pair*)b;
    if (pa->key < pb->key) return -1;
    if (pa->key > pb->key) return 1;
    return 0;
}

static int compare_uint64(void *a, void *b) {
    uint64_t ua = *(uint64_t*)a;
    uint64_t ub = *(uint64_t*)b;
    if (ua < ub) return -1;
    if (ua > ub) return 1;
    return 0;
}

typedef struct PointDct_t {
    BST *arbre;
    double xmin, xmax, ymin, ymax;
} PointDct;

PointDct* pdctCreate(List *lpoints, List *lvalues) {
    printf("pdctCreate: debut\n");
    fflush(stdout);
    
    // Vérification
    if (lpoints == NULL || listSize(lpoints) == 0) {
        printf("pdctCreate: lpoints NULL ou vide\n");
        fflush(stdout);
        return NULL;
    }
    
    printf("pdctCreate: taille lpoints = %zu\n", listSize(lpoints));
    fflush(stdout);
    
    // Allocation de la structure
    PointDct *pdct = malloc(sizeof(PointDct));
    if (pdct == NULL) {
        printf("pdctCreate: malloc failed\n");
        fflush(stdout);
        return NULL;
    }
    pdct->arbre = NULL;
    
    // Calcul des bornes avec le premier point
    LNode *curr = lpoints->head;
    if (curr == NULL) {
        printf("pdctCreate: curr NULL\n");
        fflush(stdout);
        free(pdct);
        return NULL;
    }
    
    Point *p = (Point*)curr->value;
    printf("pdctCreate: premier point (%f,%f)\n", ptGetx(p), ptGety(p));
    fflush(stdout);
    
    pdct->xmin = pdct->xmax = ptGetx(p);
    pdct->ymin = pdct->ymax = ptGety(p);
    curr = curr->next;
    
    // Parcours des points restants
    while (curr != NULL) {
        p = (Point*)curr->value;
        double x = ptGetx(p);
        double y = ptGety(p);
        
        if (x < pdct->xmin) pdct->xmin = x;
        if (x > pdct->xmax) pdct->xmax = x;
        if (y < pdct->ymin) pdct->ymin = y;
        if (y > pdct->ymax) pdct->ymax = y;
        
        curr = curr->next;
    }
    
    printf("pdctCreate: bornes x=[%f,%f] y=[%f,%f]\n", pdct->xmin, pdct->xmax, pdct->ymin, pdct->ymax);
    fflush(stdout);
    
    // Création des listes temporaires
    List *key_temp = listNew();
    List *value_temp = listNew();
    
    if (key_temp == NULL || value_temp == NULL) {
        printf("pdctCreate: key_temp ou value_temp NULL\n");
        fflush(stdout);
        if (key_temp) listFree(key_temp, false);
        if (value_temp) listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
    // Normalisation et encodage de chaque point
    curr = lpoints->head;
    LNode *currVal = lvalues->head;
    
    if (currVal == NULL) {
        printf("pdctCreate: currVal NULL\n");
        fflush(stdout);
        listFree(key_temp, true);
        listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
    int count = 0;
    while (curr != NULL) {
        count++;
        if (count % 100 == 0) printf("pdctCreate: traitement point %d\n", count);
        
        p = (Point*)curr->value;
        
        // Normalisation
        double x_norm = (ptGetx(p) - pdct->xmin) / (pdct->xmax - pdct->xmin);
        double y_norm = (ptGety(p) - pdct->ymin) / (pdct->ymax - pdct->ymin);
        
        // Conversion en uint32_t
        uint32_t X = (uint32_t)(x_norm * (double)UINT32_MAX);
        uint32_t Y = (uint32_t)(y_norm * (double)UINT32_MAX);
        
        // Encodage Morton
        uint64_t z = zEncode(X, Y);
        
        // Stockage de la clé
        uint64_t *keyPtr = malloc(sizeof(uint64_t));
        if (keyPtr == NULL) {
            printf("pdctCreate: malloc keyPtr failed\n");
            fflush(stdout);
            listFree(key_temp, true);
            listFree(value_temp, false);
            free(pdct);
            return NULL;
        }
        *keyPtr = z;
        
        listInsertLast(key_temp, keyPtr);
        listInsertLast(value_temp, currVal->value);
        
        curr = curr->next;
        currVal = currVal->next;
    }
    
    printf("pdctCreate: %d points traités\n", count);
    fflush(stdout);
    
    // ========== PARTIE MANQUANTE ==========
    size_t n = listSize(key_temp);
    printf("pdctCreate: n=%zu\n", n);
    fflush(stdout);

    Pair *pairs = malloc(n * sizeof(Pair));
    if (pairs == NULL) {
        printf("pdctCreate: malloc pairs failed\n");
        fflush(stdout);
        listFree(key_temp, true);
        listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
    // Remplir le tableau
    LNode *knode = key_temp->head;
    LNode *vnode = value_temp->head;
    for (size_t i = 0; i < n; i++) {
        pairs[i].key = *(uint64_t*)knode->value;
        pairs[i].value = vnode->value;
        knode = knode->next;
        vnode = vnode->next;
    }
    
    qsort(pairs, n, sizeof(Pair), compare_pair);
    
    listFree(key_temp, true);
    listFree(value_temp, false);
    
    key_temp = listNew();
    value_temp = listNew();
    if (key_temp == NULL || value_temp == NULL) {
        printf("pdctCreate: new lists failed\n");
        fflush(stdout);
        free(pairs);
        free(pdct);
        return NULL;
    }
    
    for (size_t i = 0; i < n; i++) {
        uint64_t *kp = malloc(sizeof(uint64_t));
        if (kp == NULL) {
            printf("pdctCreate: malloc kp failed\n");
            fflush(stdout);
            free(pairs);
            listFree(key_temp, true);
            listFree(value_temp, false);
            free(pdct);
            return NULL;
        }
        *kp = pairs[i].key;
        listInsertLast(key_temp, kp);
        listInsertLast(value_temp, pairs[i].value);
    }
    free(pairs);
    
    printf("pdctCreate: avant bstOptimalBuild, key_temp size=%zu, value_temp size=%zu\n", 
           listSize(key_temp), listSize(value_temp));
    fflush(stdout);
    
    // CONSTRUIRE l'arbre optimal
    pdct->arbre = bstOptimalBuild(compare_uint64, key_temp, value_temp);
    
    printf("pdctCreate: apres bstOptimalBuild, arbre=%p\n", (void*)pdct->arbre);
    fflush(stdout);
    
    if (pdct->arbre == NULL) {
        printf("pdctCreate: bstOptimalBuild a retourne NULL\n");
        fflush(stdout);
        listFree(key_temp, true);
        listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
    printf("pdctCreate: taille de l'arbre = %zu\n", bstSize(pdct->arbre));
    fflush(stdout);
    
    return pdct;
}

void pdctFree(PointDct *pd) {
    if (pd == NULL) return;
    if (pd->arbre != NULL) {
        bstFree(pd->arbre, true, false);  // true = libérer les clés (uint64_t*), false = ne pas libérer les valeurs
    }
    free(pd);
}

size_t pdctSize(PointDct *pd) {
    if (pd == NULL || pd->arbre == NULL) return 0;
    return bstSize(pd->arbre);
}

size_t pdctHeight(PointDct *pd) {
    if (pd == NULL || pd->arbre == NULL) return 0;
    return bstHeight(pd->arbre);
}

size_t pdctAverageNodeDepth(PointDct *pd) {
    if (pd == NULL || pd->arbre == NULL) return 0;
    return (size_t)bstAverageNodeDepth(pd->arbre);
}

void *pdctExactSearch(PointDct *pd, Point *p) {
    if (pd == NULL || pd->arbre == NULL || p == NULL) return NULL;
    
    // Normaliser le point de la requête avec les mêmes bornes
    double x_norm, y_norm;
    if (pd->xmax == pd->xmin) {
        x_norm = 0.5;
    } else {
        x_norm = (ptGetx(p) - pd->xmin) / (pd->xmax - pd->xmin);
    }
    if (pd->ymax == pd->ymin) {
        y_norm = 0.5;
    } else {
        y_norm = (ptGety(p) - pd->ymin) / (pd->ymax - pd->ymin);
    }
    
    uint32_t x_morton = (uint32_t)(x_norm * (double)UINT32_MAX);
    uint32_t y_morton = (uint32_t)(y_norm * (double)UINT32_MAX);
    uint64_t z = zEncode(x_morton, y_morton);
    
    uint64_t key_temp = z;
    return bstSearch(pd->arbre, &key_temp);
}

/*// Fonction pour obtenir le minimum de l'arbre
static BNode* bst_min(BNode *n) {
    if (n == NULL) return NULL;
    while (n->left != NULL)
        n = n->left;
    return n;
}

// Fonction pour obtenir le successeur
static BNode* bst_successor(BNode *n) {
    if (n == NULL) return NULL;
    if (n->right != NULL) {
        BNode *curr = n->right;
        while (curr->left != NULL)
            curr = curr->left;
        return curr;
    }
    BNode *y = n->parent;
    BNode *x = n;
    while (y != NULL && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y;
}
*/


// Fonctions auxiliaires
static int est_dans_cercle(double px, double py, double qx, double qy, double r) {
    double dx = px - qx;
    double dy = py - qy;
    return (dx * dx + dy * dy) <= (r * r);
}

// Convertit un code Morton (clé) en coordonnées (x, y) réelles
static void decoder_morton(uint64_t code, double *x, double *y, 
                           double xmin, double xmax, double ymin, double ymax) {
    uint32_t x_code = 0, y_code = 0;
    for (int i = 0; i < 32; i++) {
        x_code |= ((code >> (2*i)) & 1) << i;
        y_code |= ((code >> (2*i + 1)) & 1) << i;
    }
    
    *x = xmin + (double)x_code / UINT32_MAX * (xmax - xmin);
    *y = ymin + (double)y_code / UINT32_MAX * (ymax - ymin);
}

// Ajoute cette fonction avant pdctBallSearch
static void parcours_infixe(BNode *noeud, PointDct *pd, double qx, double qy, double r, List *resultat) {
    if (noeud == NULL) return;
    
    // Parcourir gauche
    parcours_infixe(noeud->left, pd, qx, qy, r, resultat);
    
    // Traiter le nœud courant
    if (noeud->key != NULL) {
        uint64_t cle_morton = *(uint64_t*)noeud->key;
        double x, y;
        decoder_morton(cle_morton, &x, &y, 
                      pd->xmin, pd->xmax, pd->ymin, pd->ymax);
        
        if (est_dans_cercle(x, y, qx, qy, r)) {
            listInsertLast(resultat, noeud->value);
        }
    }
    
    // Parcourir droite
    parcours_infixe(noeud->right, pd, qx, qy, r, resultat);
}
List *pdctBallSearch(PointDct *pd, Point *q, double r) {
    if (pd == NULL || pd->arbre == NULL || q == NULL || r < 0) {
        return NULL;
    }
    
    List *resultat = listNew();
    if (resultat == NULL) {
        return NULL;
    }
    
    double qx = ptGetx(q);
    double qy = ptGety(q);
    
    BST *arbre = (BST*)pd->arbre;
    
    parcours_infixe(((struct BST_t*)arbre)->root, pd, qx, qy, r, resultat);
    
    return resultat;
}