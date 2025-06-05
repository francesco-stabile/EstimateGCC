#!/bin/bash

echo "Test 1: File inesistente"
./estimate_algorithm 0.1 0.1 file_inesistente.dot
echo ""

echo "Test 2: File non DOT"
echo "questo non è un file DOT" > non_dot.txt
./estimate_algorithm 0.1 0.1 non_dot.txt
echo ""

echo "Test 3: Grafo vuoto"
echo "graph grafo {}" > vuoto.dot
./estimate_algorithm 0.1 0.1 vuoto.dot
echo ""

echo "Test 4: Grafo valido piccolo (3 componenti connesse)"
echo "graph grafo { a--b; c--d; e }" > piccolo.dot
./estimate_algorithm 0.1 0.1 piccolo.dot
echo ""

rm -f non_dot.txt vuoto.dot piccolo.dot
