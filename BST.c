/* ========================================================================= *
 * BST definition
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "BST.h"
#include "List.h"

/* Opaque Structure */

typedef struct BNode_t BNode;

struct BNode_t
{
    BNode *parent;
    BNode *left;
    BNode *right;
    void *key;
    void *value;
};

struct BST_t
{
    BNode *root;
    size_t size;
    int (*compfn)(void *, void *);
};

typedef struct Pair_t
{
    long sum;
    size_t n;
} Pair;

/* Prototypes of static functions */

static void bstFreeRec(BNode *n, bool freeKey, bool freeValue);
static BNode *bnNew(void *key, void *value);
static size_t bstHeightRec(BNode *root);
static BNode *bnMin(BNode *n);
static BNode *successor(BNode *n);
static double moyenne_prof(BNode *noeud, size_t profondeur_actuelle);

/* Function implementations */

BNode *bnNew(void *key, void *value)
{
    BNode *n = malloc(sizeof(BNode));
    if (n == NULL)
    {
        fprintf(stderr, "bnNew: allocation error\n");
        exit(1);
    }
    n->left = NULL;
    n->right = NULL;
    n->parent = NULL;    
    n->key = key;
    n->value = value;
    return n;
}

BST *bstNew(int comparison_fn_t(void *, void *))
{
    BST *bst = malloc(sizeof(BST));
    if (bst == NULL)
    {
        fprintf(stderr, "bstNew: allocation error");
        exit(1);
    }
    bst->root = NULL;
    bst->size = 0;
    bst->compfn = comparison_fn_t;
    return bst;
}

void bstFree(BST *bst, bool freeKey, bool freeValue)
{
    bstFreeRec(bst->root, freeKey, freeValue);
    free(bst);
}

void bstFreeRec(BNode *n, bool freeKey, bool freeValue)
{
    if (n == NULL)
        return;
    bstFreeRec(n->left, freeKey, freeValue);
    bstFreeRec(n->right, freeKey, freeValue);
    if (freeKey)
        free(n->key);
    if (freeValue)
        free(n->value);
    free(n);
}

size_t bstSize(BST *bst)
{
    return bst->size;
}

static size_t bstHeightRec(BNode *root)
{
    if (!root)
        return 0;

    size_t hleft = bstHeightRec(root->left);
    size_t hright = bstHeightRec(root->right);
    if (hleft > hright)
        return 1 + hleft;
    else
        return 1 + hright;
}

size_t bstHeight(BST *bst)
{
    if (bst->root == NULL)
        return 0;
    return bstHeightRec(bst->root) - 1;
}

bool bstInsert(BST *bst, void *key, void *value)
{
    if (bst->root == NULL)
    {
        bst->root = bnNew(key, value);
        if (bst->root == NULL)
        {
            return false;
        }
        bst->size++;
        return true;
    }
    BNode *prev = NULL;
    BNode *n = bst->root;
    while (n != NULL)
    {
        prev = n;
        int cmp = bst->compfn(key, n->key);
        if (cmp <= 0)
        {
            n = n->left;
        }
        else
        {
            n = n->right;
        }
    }
    BNode *new = bnNew(key, value);
    if (new == NULL)
    {
        return false;
    }
    new->parent = prev;
    if (bst->compfn(key, prev->key) <= 0)
    {
        prev->left = new;
    }
    else
    {
        prev->right = new;
    }
    bst->size++;
    return true;
}

void *bstSearch(BST *bst, void *key)
{
    BNode *n = bst->root;
    while (n != NULL)
    {
        int cmp = bst->compfn(key, n->key);
        if (cmp < 0)
        {
            n = n->left;
        }
        else if (cmp > 0)
        {
            n = n->right;
        }
        else
        {
            return n->value;
        }
    }
    return NULL;
}

static BNode *bnMin(BNode *n)
{
    if (n == NULL) return NULL;
    while (n->left != NULL)
        n = n->left;
    return n;
}

static BNode *successor(BNode *n)
{
    if (n == NULL) return NULL;
    if (n->right != NULL)
        return bnMin(n->right);
    BNode *y = n->parent;
    BNode *x = n;
    while (y != NULL && x == y->right)
    {
        x = y;
        y = y->parent;
    }
    return y;
}

BST *bstOptimalBuild(int comparison_fn_t(void *, void *), List *lkeys, List *lvalues)
{
    if (lkeys == NULL || listSize(lkeys) == 0) {
        return NULL;
    }

    size_t ind_milieu = listSize(lkeys) / 2;

    // Partie gauche
    List *lgauche_keys = listNew();
    List *lgauche_values = listNew();

    LNode *courant_k = lkeys->head;
    LNode *courant_v = lvalues->head;

    for (size_t i = 0; i < ind_milieu; i++) {
        listInsertLast(lgauche_keys, courant_k->value);
        listInsertLast(lgauche_values, courant_v->value);
        courant_k = courant_k->next;
        courant_v = courant_v->next;
    }
    
    void *key = courant_k->value;
    void *value = courant_v->value;

    // Partie droite
    List *ldroite_keys = listNew();
    List *ldroite_values = listNew();

    courant_k = courant_k->next;
    courant_v = courant_v->next;

    for (size_t i = 0; i < listSize(lkeys) - ind_milieu - 1; i++) {
        listInsertLast(ldroite_keys, courant_k->value);
        listInsertLast(ldroite_values, courant_v->value);
        courant_k = courant_k->next;
        courant_v = courant_v->next;
    }

    // Créer le nœud racine
    BST *bst = bstNew(comparison_fn_t);
    if (bst == NULL) {
        listFree(lgauche_keys, false);
        listFree(lgauche_values, false);
        listFree(ldroite_keys, false);
        listFree(ldroite_values, false);
        return NULL;
    }
    
    bstInsert(bst, key, value);
    
    // Récupérer les sous-arbres
    BST *gauche = bstOptimalBuild(comparison_fn_t, lgauche_keys, lgauche_values);
    BST *droite = bstOptimalBuild(comparison_fn_t, ldroite_keys, ldroite_values);
    
    // Connecter les sous-arbres
    if (gauche != NULL && gauche->root != NULL) {
        bst->root->left = gauche->root;
        gauche->root->parent = bst->root;
        bst->size += gauche->size;
    }
    
    if (droite != NULL && droite->root != NULL) {
        bst->root->right = droite->root;
        droite->root->parent = bst->root;
        bst->size += droite->size;
    }
    
    // Libérer les structures BST mais pas les nœuds
    if (gauche != NULL) {
        free(gauche);
    }
    if (droite != NULL) {
        free(droite);
    }
    
    listFree(lgauche_keys, false);
    listFree(lgauche_values, false);
    listFree(ldroite_keys, false);
    listFree(ldroite_values, false);
    
    return bst;
}

static double moyenne_prof(BNode *noeud, size_t profondeur_actuelle)
{
    if (noeud == NULL)
        return 0.0;
    
    double somme = profondeur_actuelle;
    somme += moyenne_prof(noeud->left, profondeur_actuelle + 1);
    somme += moyenne_prof(noeud->right, profondeur_actuelle + 1);
    
    return somme;
}

double bstAverageNodeDepth(BST *bst)
{
    if (bst == NULL || bst->size == 0)
        return 0.0;
    return moyenne_prof(bst->root, 0) / bst->size;
}