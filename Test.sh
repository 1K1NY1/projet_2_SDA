#!/bin/bash

echo "=== EXPERIENCE 1: Variation du nombre de points ==="
echo "Rayon fixe: 0.01, Recherches: 10000"
echo ""

for N in 10000 100000 1000000; do
    echo "--- Test avec $N points ---"
    
    echo "Liste:"
    ./testpdlist $N 10000 0.01 | grep -E "(Creation|exact|ball|Average)"
    
    echo "Arbre Morton:"
    ./testpdbst $N 10000 0.01 | grep -E "(Creation|exact|ball|Average|height|depth)"
    
    echo "Arbre 2D:"
    ./testpdbst2d $N 10000 0.01 | grep -E "(Creation|exact|ball|Average|height|depth)"
    
    echo ""
done

echo "=== EXPERIENCE 2: Variation du rayon ==="
echo "Points: 100000, Recherches: 10000"
echo ""

for R in 0.01 0.05 0.1; do
    echo "--- Test avec rayon $R ---"
    
    echo "Liste:"
    ./testpdlist 100000 10000 $R | grep -E "(Creation|exact|ball|Average)"
    
    echo "Arbre Morton:"
    ./testpdbst 100000 10000 $R | grep -E "(Creation|exact|ball|Average|height|depth)"
    
    echo "Arbre 2D:"
    ./testpdbst2d 100000 10000 $R | grep -E "(Creation|exact|ball|Average|height|depth)"
    
    echo ""
done