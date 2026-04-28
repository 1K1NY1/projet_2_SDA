#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Point.h"
#include "PointDct.h"

//Code pour arbre de recherche 2d (tout devra être static à terme)
typedef struct BNode2d_t BNode2d;
typedef struct BNode2d_t {

    BNode2d *parent;
    BNode2d *left;
    BNode2d *right;
    Point* key; //la clé est un point
    void *value;
    size_t profondeur;
};

typedef struct 
{
    BNode2d *root;
    size_t size;
}BST2d;

typedef struct PointDct_t {
    BST2d* tree;
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
    if(!root)
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
    if(!bst->root)
        return 0;
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
    Point* point;
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
    //Transforme lpoints/Lvalues en array pour qsort
    size_t nbOfPoints = lpoints->size;
    if(lpoints->size != Lvalues->size)
    {
        fprintf(stderr, "liste de points et de valeurs de taille differentes");
        return NULL;
    }
    ListPoint** arrayLPoint = malloc(sizeof(ListPoint*)*nbOfPoints);
    if(!arrayLPoint)
    {
        fprintf(stderr, "erreur d'alocation: arrayLPoint");
        return NULL;
    }
    //remplissage de arrayPoint
    LNode* n = lpoints->head;
    LNode* m = Lvalues->head;
    size_t i = 0;
    while(n)
    {
        ListPoint* lp = malloc(sizeof(ListPoint));
        if(!lp)
        {
            fprintf(stderr, "erreur d'alocation: ListPoint");
            return NULL;
        }
        lp->index = i;
        lp->point = n->value;
        lp->value = m->value;
        i++;
        n = n->next;
        m = m->next;
    }
    //Tri selon X (et remplissage)
    sort(arrayLPoint,nbOfPoints,true,compareByIndex,swap);
    for(int x = 0; x < nbOfPoints; x++)
    {
        listInsertLast(listX, arrayLPoint[x]);
    }
    //Tri selon Y (et remplissage)
    sort(arrayLPoint,nbOfPoints,false,compareByIndex,swap);
    for(int x = 0; x < nbOfPoints; x++)
    {
        listInsertLast(listY, arrayLPoint[x]);
    }
    free(arrayLPoint);

    BST2d* emptyTree = bst2dNew();
    PointDct* pd = malloc(sizeof(PointDct));
    if(!pd)
    {
        fprintf(stderr, "erreur d'allocation pour PointDct");
        return NULL;
    }
    pd->nbOfPoints = 0;
    BNode2d* root = createBst2dRec(emptyTree,NULL,listX,listY,status,0);
    emptyTree->root = root;
    pd->tree = emptyTree;
    pd->nbOfPoints = emptyTree->size;
    //Les noeuds de la liste sont vides
    free(listX);
    free(listY);
    fee(arrayLPoint);
    return pd;
}
//QuickSort
int compareByIndex(ListPoint** tab, size_t i, size_t j, bool Xsort)
{
    double c1;
    double c2;
    if(Xsort)
    {
        c1 = ptGetx(tab[i]->point);
        c2 = ptGetx(tab[j]->point);
    }
    else
    {
        c1 = ptGety(tab[i]->point);
        c2 = ptGety(tab[j]->point);
    }
    if(c1 > c2)
        return 1;
    if(c1 < c2)
        return -1;
    return 0;
}
void swap(ListPoint** tab, size_t i, size_t j)
{
    ListPoint* temp = tab[i];
    tab[i] = tab[j];
    tab[j] = temp;
}

static size_t partition(ListPoint** tab, size_t p, size_t r, bool Xsort,
                        int (*compare)(Point**, size_t i, size_t j, bool),
                        void (*swap)(Point** tableau, size_t i, size_t j))
{
    size_t i,j;
    i = p;
    for (j = p; j <= r - 1; j++) {
        if (compare(tab,j,r, Xsort) <= 0)
        {
            swap(tab, i, j);
            i = i+1;
    
        }
            
    }
    swap(tab, i, r);
    return i;
}

static void QuickSort( Point** tab, size_t p , size_t r,bool Xsort,
                int (*compare)(Point**, size_t i, size_t j, bool Xsort),
                void (*swap)(Point** tableau, size_t i, size_t j))
{
   size_t q;
   if (p < r && r != (size_t)-1)
   { 
    q = partition(tab, p, r, Xsort,compare,swap);
    if(q>0)
        QuickSort(tab, p, q-1, Xsort,compare,swap); 
    QuickSort(tab, q+1, r, Xsort,compare,swap);
   }
}

static void sort(Point** tableau, size_t length, bool Xsort,
          int (*compare)(Point**, size_t i, size_t j, bool Xsort),
          void (*swap)(Point** tableau, size_t i, size_t j)) 
{
    //Quicksort
    if (length < 2) //tableau trop petit
        return;
    QuickSort(tableau,0,length-1, Xsort,compare,swap);
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
    tree->size++;
    
    return true;
}

/*
Construit la structure PointDct de maniere optimale (recursion)
*/
static BNode2d* createBst2dRec(BST2d* tree, BNode2d* previousNode,List* splitList, List* otherList, List** status, size_t profondeur)
{
    //tableau de taille 1,2 ou 3
    if(splitList->size == 1)
    {
        BNode2d* node = bn2dNew(((ListPoint*)((splitList->last)->value))->point,((ListPoint*)((splitList->last)->value))->value,profondeur);
        node->parent = previousNode;
        node->left = NULL;
        node->right = NULL;
        //Une fois le noeud utilise on nettoie le noeud
        free(splitList->last->value); //Free listPoint
        free(splitList->last); //Free LNode
        return node;
    }
    if(splitList->size == 2)
    {
        // a securiser si necessaire
        BNode2d* node = bn2dNew(((ListPoint*)((splitList->last)->value))->point,((ListPoint*)((splitList->last)->value))->value,profondeur);
        node->parent = previousNode;
        node->left = bn2dNew(((ListPoint*)((splitList->head)->value))->point,((ListPoint*)((splitList->head)->value))->value,profondeur + 1);
        node->right = NULL;

        //Une fois le noeud utilise on nettoie le noeud
        free(splitList->last->value); //Free listPoint
        free(splitList->head->value);
        free(splitList->head); //Free LNode
        free(splitList->last);
        return node;
    }
    if(splitList->size == 3)
    {
        BNode2d* node = bn2dNew(((ListPoint*)((splitList->head->next)->value))->point,((ListPoint*)((splitList->head->next)->value))->value,profondeur);
        node->parent = previousNode;
        node->left = bn2dNew(((ListPoint*)((splitList->head)->value))->point,((ListPoint*)((splitList->head)->value))->value,profondeur + 1);
        node->right = bn2dNew(((ListPoint*)((splitList->last)->value))->point,((ListPoint*)((splitList->last)->value))->value,profondeur + 1);

        //Une fois le noeud utilise on nettoie le noeud
        free(splitList->head->next->value); //Free listPoint
        free(splitList->head->value);
        free(splitList->head->value);
        free(splitList->head->next); //Free LNode
        free(splitList->head);
        free(splitList->last);
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
    //seconde boucle pour mettre a jour status
    LNode* p = n->next;
    while(p)
    {
        status[((ListPoint*)p->value)->index] = splitListRight;
        p = p->next;
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
    while(m) 
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
    //Inverse le tableau qui est coupe a chaque nouvelle recursion x->y->x->y...
    BNode2d* node = bn2dNew(((ListPoint*)n->value)->point,((ListPoint*)n->value)->value,profondeur);
    node->parent = previousNode;
    node->left = createBst2dRec(tree,node,otherList1,splitListLeft,status,profondeur + 1);
    node->right = createBst2dRec(tree,node,otherList2,splitListRight, status, profondeur + 1);
    
    
   //une seule version des noeuds, on efface a la fin de la fonction. On doit donc uniquement effacer la structure et non les noeuds
    free(otherList1);
    free(otherList2);
    free(splitListLeft);
    free(splitListRight);
    free(n->value); //libere ListPoint
    free(n); //libère le noeud

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
    (void)pd;
    (void)p;
    (double)r;
    return NULL;
}