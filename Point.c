
#include "Point.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point_t
{
    double x;
    double y;
};

/* static functions */

#define REARTH 6371.0
#define PORTOLONG -8.6291
#define PORTOLAT 41.1579

Point *ptNewFromXY(double x, double y)
{
    Point *p = malloc(sizeof(Point));
    if (!p)
    {
        fprintf(stderr, "ptNewFromXY: Allocation of point failed\n");
        exit(EXIT_FAILURE);
    }
    p->x = x;
    p->y = y;
    return p;
}

Point *ptNewFromLL(double longitude, double latitude)
{
    double x = REARTH * M_PI * longitude / 180 * cos(PORTOLAT / 180 * M_PI);
    double y = REARTH * M_PI * latitude / 180;
    return ptNewFromXY(x, y);
}

void ptFree(Point *p)
{
    free(p);
}

double ptGetx(Point *p)
{
    return p->x;
}

double ptGety(Point *p)
{
    return p->y;
}

double ptGetLongitude(Point *p)
{
    return p->x / (REARTH * M_PI) * 180 / cos(PORTOLAT / 180 * M_PI);
}

double ptGetLatitude(Point *p)
{
    return p->y / (REARTH * M_PI) * 180;
}

double ptSqrDistance(Point *p1, Point *p2)
{
    double dx = p1->x - p2->x;
    double dy = p1->y - p2->y;

    return dx * dx + dy * dy;
}

int ptCompare(Point *p1, Point *p2)
{
    if (p1->x < p2->x)
        return -1;
    else if (p1->x > p2->x)
        return +1;
    else if (p1->y < p2->y)
        return -1;
    else if (p1->y > p2->y)
        return +1;
    else
        return 0;
}

void ptPrintXY(Point *p)
{
    printf("(%f,%f)\n", p->x, p->y);
}

void ptPrintLL(Point *p)
{
    printf("(%f,%f)\n", ptGetLongitude(p), ptGetLatitude(p));
}
