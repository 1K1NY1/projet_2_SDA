#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Point.h"
#include "PointDct.h"

//Code pour arbre de recherche 2d (tout devra être static à terme)
typedef struct BNode2d_ {

    BNode2d *parent;
    BNode2d *left;
    BNode2d *right;
    Point* key; //la clé est un point
    void *value;
    size_t profondeur;
} BNode2d;

typedef struct 
{
    BNode2d *root;
    size_t size;
}BST2d;

typedef struct PointDct_t {
    BNode2d* tree;
    size_t nbOfPoints;
} PointDct;

static BNode2d *bn2dNew(void *key, void *value, size_t profondeur)
{
    BNode2d *n = malloc(sizeof(BNode2d));
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
    n->profondeur = profondeur;
    return n;
}

static BST2d *bst2dNew()
{
    BST2d *bst = malloc(sizeof(BST2d));
    if (bst == NULL)
    {
        fprintf(stderr, "bstNew: allocation error");
        exit(1);
    }
    bst->root = NULL;
    bst->size = 0;
    return bst;
}

static void bst2dFree(BST2d *bst, bool freeKey, bool freeValue)
{
    bst2dFreeRec(bst->root, freeKey, freeValue);
    free(bst);
}

static void bst2dFreeRec(BNode2d *n, bool freeKey, bool freeValue)
{
    if (n == NULL)
        return;
    bst2dFreeRec(n->left, freeKey, freeValue);
    bst2dFreeRec(n->right, freeKey, freeValue);
    if (freeKey)
        free(n->key);
    if (freeValue)
        free(n->value);
    free(n);
}

static size_t bst2dSize(BST2d *bst)
{
    return bst->size;
}

static size_t bst2dHeightRec(BNode2d *root)
{
    if (!root)
        return 0;

    size_t hleft = bst2dHeightRec(root->left);
    size_t hright = bst2dHeightRec(root->right);
    if (hleft > hright)
        return 1 + hleft;
    else
        return 1 + hright;
}

static size_t bst2dHeight(BST2d *bst)
{
    return bst2dHeightRec(bst->root)-1;
}


static int compareBST2d(Point* key1, Point* key2, size_t profondeur)
{
    //Si profondeur est paire on compare x, si profondeur impaire on compare sur y
    if(profondeur % 2) //Impair
    {
        return compareForY(key1,key2);
    }
    return compareForX(key1,key2);
}

static int compareForX(Point* key1, Point* key2)
{
    double x1 = ptGetx(key1);
    double x2 = ptGetx(key2);
    if(x1 == x2)
        return 0;
    if(x1 > x2)
        return 1;
    return -1;
}
static int compareForY(Point* key1, Point* key2)
{
    double y1 = ptGety(key1);
    double y2 = ptGety(key2);
    if(y1 == y2)
        return 0;
    if(y1 > y2)
        return 1;
    return -1;
}


static void *bst2dSearch(BST2d *bst, void *key)
{
    BNode2d *n = bst->root;
    while (n != NULL)
    {
        int cmp = compareBST2d((Point*)key, n->key, n->profondeur);
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

//Code pour PointDct
typedef struct //structure pour optimiser la recherche.
{
    void* point;
    void* value;
    size_t index; //position dans le tableau de status
}ListPoint;

PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    //On crée deux listes liées triées selon x et y. On crée des listes de plus en plus petite en séparant la liste
    //Stockage du status des éléments
    List** status = malloc(sizeof(List*)*listSize(lpoints));
    if(!status)
    {
        fprintf(stderr, "erreur de malloc: status");
        return NULL;
    }
    List* listX = listNew();
    if(!listX)
    {
        fprintf(stderr, "erreur listX");
        return NULL;
    }
    List* listY = listNew();
    if(!listY)
    {
        fprintf(stderr, "erreur listY");
        return NULL;
    }
    //Remplis de manière triée les listes
    if(fillListX(listX,lpoints,Lvalues, status ,compareForX) == false)
    {
        fprintf(stderr, "erreur fill listX");
        return NULL;
    }
    if(fillListY(listY,lpoints,compareForY) == false)
    {
        fprintf(stderr, "erreur fill listY");
        return NULL;
    }
    BST2d* emptyTree = bst2dNew();
    PointDct* pd = malloc(sizeof(PointDct));
    if(!pd)
    {
        fprintf(stderr, "erreur d'allocation pour PointDct");
        return NULL;
    }
    pd->nbOfPoints = 0;
    pd->tree = emptyTree;
    BNode2d* root = createBst2dRec(emptyTree,NULL,listX,listY,status,0);
    emptyTree->root = root;
    pd->tree = emptyTree;
    pd->nbOfPoints = emptyTree->size;
    //libere la liste et ses elements (structure allouee ListPoint)
    listFree(listX,true);
    listFree(listY,true);
    return pd;
}

static bool fillListX(List* l, List* lpoints, List* Lvalues, List** status,int (*compare)(Point*, Point*)) //Remplis la liste X et status (par indices) de manière triée
{
    LNode* n = lpoints->head;
    LNode* val = Lvalues->head;
    //remplis la liste par un premier point
    ListPoint* p = malloc(sizeof(ListPoint));
    if(!p)
        return false;
    p->point = n->value;
    p->value = val;
    p->index = 0;

    listInsertFirst(l,n->value);
    status[0] = l;
    n = n->next;
    val = val->next;
    size_t index = 1;
    while(!n)
    {
        LNode* curr = l->head;
        if(compare((Point*)n->value, (Point*)((ListPoint*)curr->value)->point) <= 0)
        {
            p = malloc(sizeof(ListPoint));
            if(!p)
                return false;
            p->point = n->value;
            p->value = val->value;
            p->index = index;

            if(listInsertFirst(l,p) == false)
            {
                fprintf(stderr, "erreur insertion");
                return false;
            }
        }
        else
        {
            //insere a la bonne position
            p = malloc(sizeof(ListPoint));
            if(!p)
                return false;
            p->point = n->value;
             p->value = val->value;
            p->index = index;
            for(;compare((Point*)n->value, (Point*)((ListPoint*)curr->value)->point) > 0 && !curr ; curr = curr->next);
            if(listInsertAfter(l,curr,p) == false)
            {
                fprintf(stderr, "erreur insertion");
                return false;
            }
        }

        //Remplis la liste de status
        status[index] = l;

        index++;
        val = val->next;
        n = n->next;
    }
    return true;
}
static bool fillListY(List* l, List* lpoints, int (*compare)(Point*, Point*)) //Remplis la liste de manière triée
{
    LNode* n = lpoints->head;
    //remplis la liste par un premier point
    listInsertFirst(l,n->value);
    n = n->next;
    size_t index = 1;
    while(!n)
    {
        LNode* curr = l->head;
        if(compare((Point*)n->value, (Point*)curr->value) <= 0)
        {
            if(listInsertFirst(l,n->value) == false)
            {
                fprintf(stderr, "erreur insertion");
                return false;
            }
        }
        else
        {
            //insere a la bonne position
            for(;compare((Point*)n->value, (Point*)curr->value) > 0 && !curr ; curr = curr->next);
            if(listInsertAfter(l,curr,n->value) == false)
            {
                fprintf(stderr, "erreur insertion");
                return false;
            }
        }
        n = n->next;
    }
    return true;
}
//Code pour l'amelioration de la liste liee

//insere au milieu d'une liste liee
static bool listInsertAfter(List *l, LNode* prev, void *value)
{
    LNode *node = malloc(sizeof(LNode));
    if (!node) {
        fprintf(stderr, "listInsertAfter: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    node->value = value;
    node->next = NULL;
    if (!l->head)
    {
        l->head = node;
        l->last = node;
    }
    else if(l->last == prev) 
    {
        //prev est dernier
        l->last = node;
    }
    else //insere entre deux positions
    {
        node->next = prev->next;
        prev->next = node;
    }

    l->size++;
    return true;
}

//Insere a gauche ou a droite du noeud n
static bool bst2dInsert(BST2d* tree, BNode2d* n, bool left , void *key, void *value)
{
    BNode2d *new = bn2dNew(key, value, n->profondeur + 1);
    if (new == NULL)
    {
        return false;
    }

    if(left == true)
    {
        n->left = new;
    }
    else
    {
        n->right = new;
    }
    tree->size;
    
    return true;
}

/*
Construit la structure PointDct de maniere optimale (recursion)
*/
static BNode2d* createBst2dRec(BST2d* tree, BNode2d* previousNode,List* splitList, List* otherList, List** status, size_t profondeur)
{
    //tableau de taille 2 ou 3
    if(splitList->size == 2)
    {
        // a securiser si necessaire
        BNode2d* node = bn2dNew((Point*)((ListPoint*)((splitList->last)->value))->point,((ListPoint*)((splitList->last)->value))->value,profondeur);
        node->parent = previousNode;
        node->left = bn2dNew((Point*)((ListPoint*)((splitList->head)->value))->point,((ListPoint*)((splitList->head)->value))->value,profondeur);
        node->right = NULL;
        return node;
    }
    if(splitList->size == 3)
    {
        BNode2d* node = bn2dNew((Point*)((ListPoint*)((splitList->head->next)->value))->point,((ListPoint*)((splitList->head->next)->value))->value,profondeur);
        node->parent = previousNode;
        node->left = bn2dNew((Point*)((ListPoint*)((splitList->head)->value))->point,((ListPoint*)((splitList->head)->value))->value,profondeur);
        node->right = bn2dNew((Point*)((ListPoint*)((splitList->last)->value))->point,((ListPoint*)((splitList->last)->value))->value,profondeur);
        return node;
    }

    //Separe en deux
    size_t size = splitList->size;
    size_t i = 0;
    LNode* n = splitList->head;

    List* splitListLeft = listNew();
    if(!splitListLeft)
        return NULL;
    List* splitListRight = listNew();
    if(!splitListRight)
        return NULL;

    LNode* prev = n;
    while(i < size/2) //La premiere partie va dans splitListLeft
    {
        status[((ListPoint*)n->value)->index] = splitListLeft;
        prev = n;
        n = n->next;
        i++;
    }  
    status[((ListPoint*)n->value)->index] = NULL; 


    splitListLeft->head = splitList->head;
    
    //met a jour splitListRight
    splitListRight->head = n->next;
    splitListRight->last = splitList->last;
    splitListRight->size = splitList->size - (i + 1);

    //ferme splitListLeft
    prev->next = NULL;
    splitListLeft->last = prev;
    splitListLeft->size = i + 1;

    //cree les listes Y1 et Y2
    List* otherList1 = listNew();
    if(!otherList1)
    {
        fprintf(stderr,"echec de creation de liste");
        return NULL;
    } 
    List* otherList2 = listNew();
    if(!otherList2)
    {
        fprintf(stderr,"echec de creation de liste");
        return NULL;
    }
    LNode* m = otherList->head;
    while(!m) 
    {
        if(status[((ListPoint*)m->value)->index] == splitListLeft) //le noeud m est dans la nouvelle liste gauche
        {
            listInsertLast(otherList1,m->value);
        }
        else if(status[((ListPoint*)m->value)->index] == splitListRight) //evite n
        {
            listInsertLast(otherList2,m->value);
        }
        m = m->next;
    } 

    //Recursion
    //On insere n dans l'arbre
    if(profondeur == 0) //racine
    {
        BNode2d* node = malloc(sizeof(BNode2d));
        if(!node)
        {
            fprintf(stderr,"allocation de la racine a echoue");
            return NULL;
        } 
        node->parent = NULL;
        node->key = (Point*)((ListPoint*)n->value)->point;
        node->profondeur = 0;
        node->value = ((ListPoint*)n->value)->value;
        node->left = createBst2dRec(tree,node,otherList1,splitListLeft,status,1); //prochain split pour y
        node->right = createBst2dRec(tree,node,otherList2,splitList, status, 1);
        //une seule version des noeuds, on efface a la fin de la fonction. On doit donc uniquement effacer la structure et non les noeuds
        free(otherList1);
        free(otherList2);
        free(splitListLeft);
        free(splitListRight);
        free(n);
        return node; //retourne la racine (arbre complet construit)
    }
    //Inverse le tableau qui est coupe a chaque nouvelle recursion x->y->x->y...
    BNode2d* node = bn2dNew((Point*)((ListPoint*)n->value)->point,((ListPoint*)n->value)->value,profondeur);
    node->parent = previousNode;
    node->left = createBst2dRec(tree,node,otherList1,splitListLeft,status,profondeur + 1);
    node->right = createBst2dRec(tree,node,otherList2,splitList, status, profondeur + 1);
    
    
   //une seule version des noeuds, on efface a la fin de la fonction. On doit donc uniquement effacer la structure et non les noeuds
    free(otherList1);
    free(otherList2);
    free(splitListLeft);
    free(splitListRight);
    free(n);

    return node;
}


void *pdctExactSearch(PointDct *pd, Point *p) 
{
    //A changer si on veut retirer la profondeur.
    return bst2dSearch(pd->tree, p);
}

void pdctFree(PointDct *pd)
{
    (void*)pd;
    return;
}
size_t pdctSize(PointDct *pd)
{
    return pd->nbOfPoints;
}
size_t pdctAverageNodeDepth(PointDct *pd)
{
    return pd->nbOfPoints;
}
size_t pdctHeight(PointDct *pd)
{
    return pd->nbOfPoints;
}
List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
    (void*)pd;
    (void*)p;
    (double)r;
    return NULL;
}