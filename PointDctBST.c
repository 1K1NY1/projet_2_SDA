#include "BST.h"
#include "PointDct.h"
#include "Point.h"  
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

struct PointDct_t {
    BST *arbre;
    double xmin, xmax, ymin, ymax;
};

typedef struct {
    Point *pt;
    void *value;
} PointValue;

typedef struct {
    uint64_t key;
    void *value;
} Pair;

//declarations
static uint64_t interleave8(uint8_t m, uint8_t n);
static uint64_t zEncode(uint32_t x, uint32_t y);
static int compare_pair(const void *a, const void *b);
static int compare_uint64(void *a, void *b);

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
    if (curr == NULL) {
        free(pdct);
        return NULL;
    }

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

    if (currVal == NULL) {
        listFree(key_temp, true);
        listFree(value_temp, false);
        free(pdct);
        return NULL;
    }

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
            fprintf(stderr,"pdctCreate: malloc keyPtr failed\n");
            listFree(key_temp, true);
            listFree(value_temp, false);
            free(pdct);
            return NULL;
        }
        *keyPtr = z;

        // On enrobe le Point et la valeur utilisateur dans un PointValue
        // pour pouvoir retrouver la position lors des recherches.
        PointValue *pv = malloc(sizeof(PointValue));
        if (pv == NULL) {
            free(keyPtr);
            listFree(key_temp, true);
            listFree(value_temp, true);
            free(pdct);
            return NULL;
        }
        pv->pt = p;
        pv->value = currVal->value;

        listInsertLast(key_temp, keyPtr);
        listInsertLast(value_temp, pv);

        curr = curr->next;
        currVal = currVal->next;
    }

    size_t n = listSize(key_temp);

    Pair *pairs = malloc(n * sizeof(Pair));
    if (pairs == NULL) {
        listFree(key_temp, true);
        listFree(value_temp, true);
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

    pdct->arbre = bstOptimalBuild(compare_uint64, key_temp, value_temp);

    listFree(key_temp, false);
    listFree(value_temp, false);

    if (pdct->arbre == NULL) {
        free(pdct);
        return NULL;
    }

    return pdct;
}

void pdctFree(PointDct *pd) {
    if (pd == NULL) return;
    if (pd->arbre != NULL) {
        // freeKey=true libere les uint64_t* alloues dans pdctCreate.
        // freeValue=true libere les PointValue* (mais ni le Point ni la
        // valeur utilisateur qu'ils contiennent, qui appartiennent a l'appelant).
        bstFree(pd->arbre, true, true);
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

    // Plusieurs points peuvent avoir le meme code. 
    // on prends tout les candidats puis on trouve celui qui correspond
    List *candidats = bstRangeSearch(pd->arbre, &z, &z);
    if (candidats == NULL) return NULL;

    void *resultat = NULL;
    for (LNode *n = candidats->head; n != NULL; n = n->next) {
        PointValue *pv = (PointValue*)n->value;
        if (ptCompare(pv->pt, p) == 0) {
            resultat = pv->value;
            break;
        }
    }

    listFree(candidats, false);
    return resultat;
}

static uint32_t normaliser_u32(double v, double vmin, double vmax) {

    if (vmax == vmin)
        return 0;

    double norm = (v - vmin) / (vmax - vmin);

    if (norm < 0.0) 
        norm = 0.0;
    if (norm > 1.0) 
        norm = 1.0;

    return (uint32_t)(norm * (double)UINT32_MAX);
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

    // carre qui englobe la boule
    double xmin_b = qx - r;
    double xmax_b = qx + r;
    double ymin_b = qy - r;
    double ymax_b = qy + r;

    // Si la boule est entierement en dehors des donnees: renvoie une liste vide
    if (xmax_b < pd->xmin || xmin_b > pd->xmax || ymax_b < pd->ymin || ymin_b > pd->ymax)
        return resultat;
    
    // Normaliser et calculer les codes Morton des coins
    uint32_t x_lo = normaliser_u32(xmin_b, pd->xmin, pd->xmax);
    uint32_t x_hi = normaliser_u32(xmax_b, pd->xmin, pd->xmax);
    uint32_t y_lo = normaliser_u32(ymin_b, pd->ymin, pd->ymax);
    uint32_t y_hi = normaliser_u32(ymax_b, pd->ymin, pd->ymax);

    uint64_t z_min = zEncode(x_lo, y_lo);
    uint64_t z_max = zEncode(x_hi, y_hi);

    List *candidats = bstRangeSearch(pd->arbre, &z_min, &z_max);
    if (candidats == NULL) {
        listFree(resultat, false);
        return NULL;
    }

    double r2 = r * r;
    for (LNode *n = candidats->head; n != NULL; n = n->next) {
        PointValue *pv = (PointValue*)n->value;
        if (ptSqrDistance(pv->pt, q) <= r2) {
            listInsertLast(resultat, pv->value);
        }
    }

    listFree(candidats, false);
    return resultat;
}