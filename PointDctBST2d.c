#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Point.h"
#include "PointDct.h"

//structures
typedef struct BNode2d_t BNode2d;
struct BNode2d_t {

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

struct PointDct_t {
    BST2d* tree;
    size_t nbOfPoints;
};

typedef struct //structure pour optimiser la recherche.
{
    Point* point;
    void* value;
    size_t index; //position dans le tableau de status
}ListPoint;

//Type array
typedef struct
{
    ListPoint** list;
    size_t length;
}Array;


//declarations
static BNode2d *bn2dNew(void *key, void *value, size_t profondeur);
static BST2d *bst2dNew();
static void bst2dFree(BST2d *bst, bool freeKey, bool freeValue);
static void bst2dFreeRec(BNode2d *n, bool freeKey, bool freeValue);
static size_t bst2dSize(BST2d *bst);
static size_t bst2dHeightRec(BNode2d *root);
static size_t bst2dHeight(BST2d *bst);
static int compareBST2d(Point* key1, Point* key2, size_t profondeur);
static int compareForX(Point* key1, Point* key2);
static int compareForY(Point* key1, Point* key2);
static void *bst2dSearch(BST2d *bst, void *key);
static size_t sumOfDepthRec(BNode2d* n);

static Array* newArrayFromArray(size_t start, size_t end, Array* arr, Array** status);
static Array* newArrayAs(Array* arr);
static Array* newArrayFromListPoint(ListPoint** list, size_t length, bool update, Array** status);
static void freeArray(Array* arr);

static int compareByIndex(ListPoint** tab, size_t i, size_t j, bool Xsort);
static void swap(ListPoint** tab, size_t i, size_t j);
static size_t partition(ListPoint** tab, size_t p, size_t r, bool Xsort,
                        int (*compare)(ListPoint**, size_t i, size_t j, bool),
                        void (*swap)(ListPoint** tableau, size_t i, size_t j));
static void QuickSort( ListPoint** tab, size_t p , size_t r,bool Xsort,
                int (*compare)(ListPoint**, size_t i, size_t j, bool Xsort),
                void (*swap)(ListPoint** tableau, size_t i, size_t j));
static void sort(ListPoint** tableau, size_t length, bool Xsort,
          int (*compare)(ListPoint**, size_t i, size_t j, bool Xsort),
          void (*swap)(ListPoint** tableau, size_t i, size_t j));

static bool bst2dInsert(BST2d* tree, BNode2d* n, bool left , void *key, void *value);
static BNode2d* createBst2dRec(BST2d* tree, BNode2d* previousNode,Array* splitList, Array* otherList, Array** status, size_t profondeur);
static bool ballSearchRec(List* result, BNode2d* n, Point *p, double r);


//Code pour arbre de recherche 2d (tout devra être static à terme)


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

static size_t sumOfDepthRec(BNode2d* n)
{
    if(!n)
        return 0;
    return sumOfDepthRec(n->left) + sumOfDepthRec(n->right) + n->profondeur;
}

//Code pour PointDct


//Copie un array à partir d'un autre (de start à end compris), mets à jour statut
static Array* newArrayFromArray(size_t start, size_t end, Array* arr, Array** status)
{
    Array* res = malloc(sizeof(Array));
    if(!res)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    size_t size = end - start + 1; //Contient end
    ListPoint** l = malloc(sizeof(ListPoint*)*size);
    if(!l)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    for(size_t i = 0; i < size; i++)
    {
        l[i] = arr->list[start + i];
        status[arr->list[start + i]->index] = res;
    }
    res->list = l;
    res->length = size;
    return res;
}

//cree un array de meme taille que arr
static Array* newArrayAs(Array* arr)
{
    size_t size = arr->length;
    Array* res = malloc(sizeof(Array));
    if(!res)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    ListPoint** l = malloc(sizeof(ListPoint*)*size);
    if(!l)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    res->list = l;
    res->length = size;
    return res;
}
//cree un array de taille length à partir d'une liste de ListPoint
static Array* newArrayFromListPoint(ListPoint** list, size_t length, bool update, Array** status)
{
    Array* res = malloc(sizeof(Array));
    if(!res)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    ListPoint** l = malloc(sizeof(ListPoint*)*length);
    if(!l)
    {
        fprintf(stderr, "erreur d'allocation pour Array");
        return NULL;
    }
    for(size_t i = 0; i < length; i++)
    {
        l[i] = list[i];
        if(update)
            status[list[i]->index] = res;
    }
    res->list = l;
    res->length = length;
    return res;
}
static void freeArray(Array* arr)
{
    free(arr->list);
    free(arr);
    return;
}

PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    //On crée deux listes liées triées selon x et y. On crée des listes de plus en plus petite en séparant la liste
    //Stockage du status des éléments
    Array** status = malloc(sizeof(Array*)*listSize(lpoints));
    if(!status)
    {
        fprintf(stderr, "erreur de malloc: status");
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

    //remplissage de arrayLPoint
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
        arrayLPoint[i] = lp;
        i++;
        n = n->next;
        m = m->next;
    }

    //tri par X
    sort(arrayLPoint,nbOfPoints,true,compareByIndex,swap);
    Array* listX = newArrayFromListPoint(arrayLPoint,nbOfPoints,true,status);
    if(!listX)
    {
        fprintf(stderr, "erreur listX");
        return NULL;
    }
    //tri par Y
    sort(arrayLPoint,nbOfPoints,false,compareByIndex,swap);
    Array* listY = newArrayFromListPoint(arrayLPoint,nbOfPoints,false,status);
    if(!listY)
    {
        fprintf(stderr, "erreur listY");
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
    BNode2d* root = createBst2dRec(emptyTree,NULL,listX,listY,status,0);
    if(!root)
    {
        fprintf(stderr, "echec de la creation de l'arbre");
        return NULL;
    }
    emptyTree->root = root;
    pd->tree = emptyTree;
    pd->nbOfPoints = nbOfPoints;
    //Les noeuds de la liste sont vides
    //Libère les listPoints
    for(size_t ind = 0; ind < nbOfPoints; ind++)
    {
        free(arrayLPoint[ind]);
    }
    //libère les arrays
    freeArray(listX);
    freeArray(listY);
    free(arrayLPoint);
    free(status);
    return pd;
}
//QuickSort
static int compareByIndex(ListPoint** tab, size_t i, size_t j, bool Xsort)
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
static void swap(ListPoint** tab, size_t i, size_t j)
{
    ListPoint* temp = tab[i];
    tab[i] = tab[j];
    tab[j] = temp;
}

static size_t partition(ListPoint** tab, size_t p, size_t r, bool Xsort,
                        int (*compare)(ListPoint**, size_t i, size_t j, bool),
                        void (*swap)(ListPoint** tableau, size_t i, size_t j))
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

static void QuickSort( ListPoint** tab, size_t p , size_t r,bool Xsort,
                int (*compare)(ListPoint**, size_t i, size_t j, bool Xsort),
                void (*swap)(ListPoint** tableau, size_t i, size_t j))
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

static void sort(ListPoint** tableau, size_t length, bool Xsort,
          int (*compare)(ListPoint**, size_t i, size_t j, bool Xsort),
          void (*swap)(ListPoint** tableau, size_t i, size_t j)) 
{
    //Quicksort
    if (length < 2) //tableau trop petit
        return;
    QuickSort(tableau,0,length-1, Xsort,compare,swap);
}

/*Non utilisé
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
*/

/*
Construit la structure PointDct de maniere optimale (recursion)
*/
static BNode2d* createBst2dRec(BST2d* tree, BNode2d* previousNode,Array* splitList, Array* otherList, Array** status, size_t profondeur)
{
    //tableau de taille 1,2 ou 3
    if(splitList->length == 1)
    {
        BNode2d* node = bn2dNew(splitList->list[0]->point,splitList->list[0]->value,profondeur);
        tree->size++;
        node->parent = previousNode;
        node->left = NULL;
        node->right = NULL;
        freeArray(splitList);
        freeArray(otherList);
        return node;
    }
    if(splitList->length == 2)
    {
        BNode2d* node = bn2dNew(splitList->list[1]->point,splitList->list[1]->value,profondeur);
        
        node->parent = previousNode;
        node->left = bn2dNew(splitList->list[0]->point,splitList->list[0]->value,profondeur + 1);
        node->right = NULL;
        tree->size += 2;
        freeArray(splitList);
        freeArray(otherList);
        return node;
    }
    if(splitList->length == 3)
    {
        BNode2d* node = bn2dNew(splitList->list[1]->point,splitList->list[1]->value,profondeur);
        node->parent = previousNode;
        node->left = bn2dNew(splitList->list[0]->point,splitList->list[0]->value,profondeur + 1);
        node->right = bn2dNew(splitList->list[2]->point,splitList->list[2]->value,profondeur + 1);
        tree->size += 3;
        freeArray(splitList);
        freeArray(otherList);
        return node;
    }

    //Separe en deux
    size_t size = splitList->length;

    Array* splitListLeft = newArrayFromArray(0,size/2-1,splitList, status);
    if(!splitListLeft)
        return NULL;
    Array* splitListRight = newArrayFromArray(size/2 + 1,size-1,splitList, status);
    if(!splitListRight)
        return NULL;

    ListPoint* n = splitList->list[size/2];
    status[n->index] = NULL; 

    //cree les listes Y1 et Y2
    Array* otherList1 = newArrayAs(splitListLeft);
    if(!otherList1)
    {
        fprintf(stderr,"echec de creation de liste");
        return NULL;
    } 
    Array* otherList2 = newArrayAs(splitListRight);
    if(!otherList2)
    {
        fprintf(stderr,"echec de creation de liste");
        return NULL;
    }
    
    //remplis otherList1 et otherList2;

    for(size_t x = 0, y = 0, j = 0; j < otherList->length; j++) 
    {
        if(status[otherList->list[j]->index] == splitListLeft) //le noeud m est dans la nouvelle liste gauche
        {
            otherList1->list[x++] = otherList->list[j];
        }
        else if(status[otherList->list[j]->index] == splitListRight) //evite n
        {
            otherList2->list[y++] = otherList->list[j];
        }
    } 

    //On libère les Array précédent car des copies en ont été faites (aucun code ne l'utilise a nouveau)
    freeArray(splitList);
    freeArray(otherList);
    //Recursion
    //On insere n dans l'arbre
    //Inverse le tableau qui est coupe a chaque nouvelle recursion x->y->x->y...
    BNode2d* node = bn2dNew(n->point,n->value,profondeur);
    tree->size++;
    node->parent = previousNode;
    node->left = createBst2dRec(tree,node,otherList1,splitListLeft,status,profondeur + 1);
    node->right = createBst2dRec(tree,node,otherList2,splitListRight, status, profondeur + 1);
    
    return node;
}


void *pdctExactSearch(PointDct *pd, Point *p) 
{
    return bst2dSearch(pd->tree, p);
}

void pdctFree(PointDct *pd)
{
    bst2dFree(pd->tree,false,false);
    free(pd);
    return;
}
size_t pdctSize(PointDct *pd)
{
    return pd->nbOfPoints;
}
size_t pdctAverageNodeDepth(PointDct *pd)
{
    if(!pd || pd->nbOfPoints == 0)
        return 0;
    size_t sum = sumOfDepthRec(pd->tree);
    size_t average = sum/pd->nbOfPoints;
    return average;
}
size_t pdctHeight(PointDct *pd)
{
    return bst2dHeight(pd->tree);
}
List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
    List* result = listNew();
    if(!result)
    {
        fprintf(stderr, "echec d'allocation de liste");
        return NULL;
    }
    if(!ballSearchRec(result, pd->tree->root,p,r))
    {
        fprintf(stderr, "echec durant la recursion de ballSearch");
        return NULL;
    }
    return result;
}

static bool ballSearchRec(List* result, BNode2d* n, Point *p, double r)
{
    if(!n)
        return true;
    
    size_t profondeur = n->profondeur;
    if(ptSqrDistance(n->key,p) <= r*r) //le point fait partie de la boule
    {
        if(!listInsertLast(result,n->value)) //ajoute la valeur a la liste
            return false;
    }
    //choix du parcours
    double v_n, v_p;
    if(profondeur % 2) //impair (y)
    {
        v_n = ptGety(n->key);
        v_p = ptGety(p);
    }
    else //pair (x)
    {
        v_n = ptGetx(n->key);
        v_p = ptGetx(p);
    }
    if(v_p - r < v_n) //on explore le sous-arbre de gauche
    {
        if(!ballSearchRec(result,n->left,p,r))
            return false;
    }
    if(v_p + r >= v_n) //on explore le sous-arbre de droite
    {
        if(!ballSearchRec(result,n->right,p,r))
            return false;
    }
    return true;
}