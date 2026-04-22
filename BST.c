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

typedef struct kvpair_t
{
    void *key;
    void *value;
} KVpair;

/* Prototypes of static functions */

static void bstFreeRec(BNode *n, bool freeKey, bool freeValue);
static BNode *bnNew(void *key, void *value);
static size_t bstHeightRec(BNode *root);
static BNode *bnMin(BNode *n);
static BNode *successor(BNode *n);

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
    bn->parent = NULL;    
    n->key = key;
    n->value = value;
    return n;
}

BST *bstNew(int comparison_fn_t(void *, void *))
{
    BST *bst = malloc(sizeof(BST));
    if (bst == NULL)
    {
        fprintf(stderr, "bestNew: allocation error");
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
    return bstHeightRec(bst->root)-1;
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
        else if (cmp > 0)
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
/* static BNode *bnMin(BNode *n)
{
    while (n->left != NULL)
        n = n->left;
    return n;
}

static BNode *successor(BNode *n)
{
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
 */

// ----------------------------------------------------------------------------------
// The functions below have to be implemented

BST *bstOptimalBuild(int comparison_fn_t(void *, void *), List *lkeys, List *lvalues)
{
    // architecture d'un arbre binaire de recherche dans la quelle c'est l'indice du millieu qui est 
    //  la clé, ce qui nous permeet de trouver les indices de façon éfficace 
    if (lkeys == NULL || listSize(lkeys) == 0) // si la 
    return NULL;

    size_t ind_milieu = listSize(lkeys)/2;


    //partie gauche 

    List *lgauche_keys = listNew();
    List *lgauche_values = listNew();

    LNode *courant_k = lkeys ->head; 
    LNode *courant_v = lvalues ->head; 

    for (size_t i =0 ; i<ind_milieu;i++){
        listInsertLast(lgauche_keys,courant_k->value);
        listInsertLast(lgauche_values,courant_v->value);
        courant_k = courant_k -> next;
        courant_v = courant_v -> next;
    }
    void *key = courant_k->value;
    void *value =  courant_v->value; 

    //partie droit

    List *ldroite_keys = listNew();
    List *ldroite_values = listNew();

    courant_k = courant_k ->next;
    courant_v =  courant_v->next ;

    for (size_t i =0 ; i<listSize(lkeys)-ind_milieu -1 ;i++){
        listInsertLast(ldroite_keys,courant_k->value);
        listInsertLast(ldroite_values,courant_v->value);
        courant_k = courant_k -> next;
        courant_v = courant_v -> next;
    }

    BST *bst = bstNew(comparison_fn_t);
    bstInsert(bst , key ,value);


    //récursive

    BST *gauche = bstOptimalBuild(comparison_fn_t , lgauche_keys , lgauche_values);
    BST *droite = bstOptimalBuild(comparison_fn_t, ldroite_keys ,ldroite_values);
    

    if (gauche != NULL)
        bst->root->left = gauche->root;
   
    if(droite != NULL)
        bst->root->right = droite->root;


    bstFree(gauche, false, false);
    bstFree(droite, false, false);
    listFree(lgauche_keys, false);
    listFree(lgauche_values, false);
    listFree(ldroite_keys, false);
    listFree(ldroite_values, false);

    return bst;
}
static void rangeSearchRec(BNode *noeud, BST *bst, void *keymin, void *keymax, List *result)
{
    if (noeud == NULL)
        return;

    int compmin = bst->compfn(noeud->key, keymin);
    int compmax = bst->compfn(noeud->key, keymax);

    if (compmin < 0)
    {
        rangeSearchRec(noeud->right, bst, keymin, keymax, result);
    }
    else if (compmax > 0)
    {
        rangeSearchRec(noeud->left, bst, keymin, keymax, result);
    }
    else
    {
        rangeSearchRec(noeud->left, bst, keymin, keymax, result);
        listInsertLast(result, noeud->value);
        rangeSearchRec(noeud->right, bst, keymin, keymax, result);
    }
}

List *bstRangeSearch(BST *bst, void *keymin, void *keymax)
{
    if (bst == NULL)
        return NULL;

    List *result = listNew();
    if (result == NULL)
        return NULL;

    rangeSearchRec(bst->root, bst, keymin, keymax, result);
    return result;
}

double bstAverageNodeDepth(BST *bst)
{
    // To implement
    return 0.0;
}
