#include "BST.h"
#include "PointDct.h"
#include "Point.h"  
#include <stdlib.h>
#include <stdint.h>


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

typedef struct PointDct_t {
    BST *arbre;
    double xmin, xmax, ymin, ymax;
} PointDct;


PointDct* pdctCreate(List *lpoints, List *lvalues) {
    // Vérification
    if (lpoints == NULL || listSize(lpoints) == 0) {
        return NULL;
    }
    
    // Allocation de la structure
    PointDct *pdct = malloc(sizeof(PointDct));
    if (pdct == NULL) {
        return NULL;
    }
    pdct->arbre = NULL;
    
    // Calcul des bornes avec le premier point
    LNode *curr = lpoints->head;
    Point *p = (Point*)curr->value;
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
    
    // Création des listes temporaires
    List *key_temp = listNew();
    List *value_temp = listNew();
    
    if (key_temp == NULL || value_temp == NULL) {
        if (key_temp) listFree(key_temp, false);
        if (value_temp) listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
    // Normalisation et encodage de chaque point
    curr = lpoints->head;
    LNode *currVal = lvalues->head;
    
    while (curr != NULL) {
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
    size_t n = listSize(key_temp);
    
    typedef struct {
        uint64_t key;
        void *value;
    } Pair;
    
    Pair *pairs = malloc(n * sizeof(Pair));
    if (pairs == NULL) {
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
    
    static int compare_pair(const void *a, const void *b){
        Pair *pa = (Pair*)a;
        Pair *pb = (Pair*)b;
        if (pa->key < pb->key) return -1;
        if (pa->key > pb->key) return 1;
        return 0;
    }
    
    qsort(pairs, n, sizeof(Pair), compare_pair);
    
    
    listFree(key_temp, true);   // libère les anciennes clés
    listFree(value_temp, false); // ne libère pas les valeurs
    
    key_temp = listNew();
    value_temp = listNew();
    if (key_temp == NULL || value_temp == NULL) {
        free(pairs);
        free(pdct);
        return NULL;
    }
    
    for (size_t i = 0; i < n; i++) {
        uint64_t *kp = malloc(sizeof(uint64_t));
        if (kp == NULL) {
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

    // Fonction de comparaison pour clés uint64_t (à mettre en haut)
    static int compare_uint64(void *a, void *b) {
        uint64_t ua = *(uint64_t*)a;
        uint64_t ub = *(uint64_t*)b;
        if (ua < ub) return -1;
        if (ua > ub) return 1;
        return 0;
    }
    
    // CONSTRUIRE l'arbre optimal
    pdct->arbre = bstOptimalBuild(compare_uint64, key_temp, value_temp);
    if (pdct->arbre == NULL) {
        listFree(key_temp, true);
        listFree(value_temp, false);
        free(pdct);
        return NULL;
    }
    
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


List *pdctBallSearch(PointDct *pd, Point *q, double r) {
    // Cette fonction est plus complexe car un rayon en 2D ne correspond pas
    // à un simple intervalle en code de Morton.
    // Pour l'instant, on retourne NULL (à implémenter)
    (void)pd;
    (void)q;
    (void)r;
    return NULL;
}