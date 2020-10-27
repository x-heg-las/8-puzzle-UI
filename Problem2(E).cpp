
#include <iostream>
#include <queue>
#include "Problem2H.h"
#include <unordered_map>
#include<stdio.h>
#include <chrono>
#include <string>
#include<string.h>
#pragma pack(1)

using namespace Hlavolam;
/*
    Funkcia, ktorá testuje, èi je daný uzol koncový
*/
bool Node::isLast() {
    
    char width, height;
    width = map->width;
    height = map->height;

    for (int i = 0; i < height; i++) {
        if (memcmp(state[i], map->targetNode->state[i], width)) // porovnáva jednotlivé polia stavov v 2D poli
            return false;
    }

    return true;
}

/*
    Heiristika 1: Funkcia vàati poèet nesprávne umiestnených políèok v hlavolame

    Node* current - skumany uzol
*/
unsigned char Heuristic::heuristic1(Node* current) {

    char width, height;
    width = current->map->width;
    height = current->map->height;
    unsigned int misplaced = 0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (current->state[i][j] ^ current->map->targetNode->state[i][j])
                misplaced++;
        }
    }

    return misplaced;
}

/*
    Pomocná funkcia pre heuristiku 2 (Manhattanská cesta). Funkcia je použitá
    pre h¾adanie pozície jednotlivých znakov/èísel v hlavolame.
*/
void findCoord(char** arr, int width, int height, char find, int* x, int* y) {
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (arr[i][j] == find)
            {
                *x = j;
                *y = i;
                return;
            }
        }
    }
}

/*
    Heuristická funkcia poèítajúca súèet Manhattanských ciest jednotlivých políèok hlavolamu

    Node* current - skumany uzol
*/
unsigned char Heuristic::heuristic2(Node* current) {
    char width, height;
    width = current->map->width;
    height = current->map->height;
    int X = 0, Y = 0;
    unsigned int sum = 0;
    char** tar = current->map->targetNode->state;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (current->state[i][j]) {
                findCoord(tar, width, height, current->state[i][j], &X, &Y);
                sum += abs(X - j) + abs(Y - i);
            }
        }
    }
    return sum;
}

/*
    Funkcia pod¾a argumentu nastaví požadovanú heuristickú funkciu pre vykonanie algoritmu.
*/
void Playground::setHeuristic(Heuristics function) {
    
    heuristicFunction = function;
}

/*
    Konštruktor triedy Playground predstavujúcej inštanciu hlavolamu, ktorýobsahuje informácie o rozmeroch hlavolamu a
    koncovému stavu.
*/
Playground::Playground(int width_, int height_, char** targetConfiguration) {
    width = width_; // sirka hlavolamu
    height = height_; // vyska hlavolamu
    targetNode = new Node(targetConfiguration, width, height, nullptr,100,0, this); 
    
}

/*
    Funkcia vráti súèet håbky uzla a heuristickej funkcie použitej na daný uzol
*/
unsigned int Node::score() {
    
    return G + H;
}


/*
    Funkcia premení pole predstavujúce stav na èíslo, ktoré je použité ako k¾uè v 
    hash tabu¾ke.
    Ak by bolo pole reprezentované nasledovne {1,5,4,3}, tak funkcia vráti èíslo 1543.

    char** state - stav uzla
    char width, char height - rozmery hlavolamu
*/
ulli getIntVal(char** state, char width, char height) {
    ulli cost = 0;
    
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            cost += state[i][j];
            cost *= 10;
        }
    }

    return cost;
}

/*
    Zistí, èi bol už daný stav vytvorený. Ak nebol, stav je vložený do dátovej štruktúry obsahujúcej objaveé stavy.
    Ak stav už bol vytvorený, ale jeho cena je menšia ako cena stavu skôr vytvoreného, tento uzol je vložený do množiny. 

    Node* node - skúmaný uzol
    std::unordered_map<ulli, Node*> &closedSet - hash tabulka s vytvorenymi uzlami
    char** newConfig - skúmaná konfigurácia
    char width, char height - rozmery hlavolamu
*/
bool colision(Node* node,std::unordered_map<ulli, Node*> &closedSet, char** newConfig, char width, char hight) {
    
    ulli input = getIntVal(newConfig, width, hight);

    std::unordered_map<ulli,Node*>::const_iterator got = closedSet.find(input);

    if (got == closedSet.end() || got->second->score() > node->score()) {
        
        closedSet[input] = node;
        return true;
    }
        
    return false;
}

/*
    Funkcia použitá pre uvolnenie alokovanej pamäti pre uzly

    Node* n - skúmaný uzol
    int height - výška hlavolamu
*/
void deleteNode(Node* n, int height) {

    for (int i = 0; i < height; i++) {
        delete[] n->state[i];
    }
    delete n->state;
    delete n;
}

void movement(Node* parentNode, openSet* queue, std::unordered_map<ulli, Node*> *closedSet, char** state){

    unsigned int height, width;
    height = parentNode->map->height;
    width = parentNode->map->width;
    char X_ = -1 , Y_ = -1;
    char copy = 0;

    for (unsigned int i = 0; i < height; i++) {

        for (unsigned int j = 0; j < width && !copy; j++) {
            
            if (!copy && parentNode->state[i][j] ==0) {
                X_ = j;
                Y_ = i;
                copy = 1;
                
            }
            state[i][j] = parentNode->state[i][j];

        }
        if (copy) {
            memcpy(state[i], parentNode->state[i], width);
        }
    }

    Node* newNode = NULL;
    if (X_ + 1 < width  && parentNode->operation != RIGHT) {
        // Left
        state[Y_][X_] = state[Y_][X_ + 1];
        state[Y_][X_ + 1] = 0;
        newNode = new Node(state, width, height, parentNode, LEFT, parentNode->G + 1, parentNode->map);
        newNode->H = parentNode->map->heuristicFunction(newNode);
        if (colision(newNode, *closedSet, state, width, height))
            queue->push(newNode);
        else
            deleteNode(newNode, height);
        state[Y_][X_ + 1] = state[Y_][X_];
        state[Y_][X_] = 0;
    }
    if (X_ - 1 >= 0 && parentNode->operation != LEFT) {
        //Right
        state[Y_][X_] = state[Y_][X_ - 1];
        state[Y_][X_ - 1] = 0;
        newNode = new Node(state, width, height, parentNode, RIGHT, parentNode->G + 1, parentNode->map);
        newNode->H = parentNode->map->heuristicFunction(newNode);
        if (colision(newNode, *closedSet, state, width, height))
            queue->push(newNode);
        else
            deleteNode(newNode, height);
        state[Y_][X_ - 1] = state[Y_][X_];
        state[Y_][X_] =0;
    }
    if (Y_ + 1 <  height && parentNode->operation != DOWN) {
        //Up
        state[Y_][X_] = state[Y_ + 1][X_];
        state[Y_ + 1][X_] = 0;
        newNode = new Node(state, width, height, parentNode, UP, parentNode->G + 1, parentNode->map);
        newNode->H = parentNode->map->heuristicFunction(newNode);
        if (colision(newNode, *closedSet, state, width, height))
            queue->push(newNode);
        else
            deleteNode(newNode, height);
        state[Y_ + 1][X_] = state[Y_][X_];
        state[Y_][X_] =0;
    }
    if (Y_ - 1 >= 0 && parentNode->operation != UP) {
        //Down
        state[Y_][X_] = state[Y_ - 1][X_];
        state[Y_ - 1][X_] = 0;
        newNode = new Node(state, width, height, parentNode, DOWN, parentNode->G + 1, parentNode->map);
        newNode->H = parentNode->map->heuristicFunction(newNode);
        if (colision(newNode, *closedSet, state, width, height))
            queue->push(newNode);
        else
            deleteNode(newNode, height);
        state[Y_ - 1][X_] = state[Y_][X_];
        state[Y_][X_] = 0;
    }

}

/*
    Funkcia predstavujúca jadro vykonávaného A* algoritmu. 
    Parameter startNode predstavuje poèiatoèný stav hlavolamu, na ktorý sú postupne
    aplikované operácie pre nájdenie nových uzlov/stavov.

    char** startNode - poèiatoèný stav
*/
std::unordered_map<ulli, Node*> Playground::findCombination(char** startNode) {

    // Prioritný rad, do ktorého sú pridávané uzly, ktoré ešte neboli preskúmané
    openSet untravelled; // std::priority_queue<unsigned long long int, Node*>  

    std::unordered_map<ulli, Node*> closedSet; //dátova štruktura obsahujuca všetky vytvorené uzly

    Node* curNode =  new Node(startNode, width, height, nullptr, 0, 0,this); //Vytvorenie uzla obsahujúci poèiatoèný stav
    
    untravelled.push(curNode);// vloženie poèiatoèného uzla do prioritného radu
    closedSet[getIntVal(curNode->state, width, height)] = curNode;// vloženie poèiatoèného uzla do vytvorených uzlov
    
   
    while (!untravelled.empty()) { //cyklus iterujúci dokial nie je prioritný rad prázdny
        
        curNode = untravelled.top(); 

        if (curNode->isLast()) { // ak je aktuálne skúmaný stav totožný s cie¾ovým, funkcia vypíše celkový poèet objavených uzlov
            std::cout << "Pocet vygenerovanych uzlov : " << closedSet.size() << std::endl;
            targetNode = curNode;
            return closedSet;
        }

        untravelled.pop(); //odobratie uzla z prioritného radu s najnižšou cenou
       

        movement(curNode, &untravelled, &closedSet, startNode); //funkcia vykonávajúce operácie nad skúmaným uzlom (stavom)
    }

    std::cout << "Pocet vygenerovanych uzlov : " << closedSet.size()<< std::endl;
    targetNode = NULL;

    return closedSet;
    
}

/*
    Funkcia rekurzívne skonštruuje ceestu od poèiatoèného ku koneènému uzlu a vypíše
    operátory

    int* op pole reprezentujuce ciselne hodnoty operátorov od zaciatku po koniec
*/

void printOperators(Node* n, int *op) {
    
    if (n->parent != NULL)
        printOperators(n->parent, op);

    switch(n->operation)
    {
    case -1:
        std::cout << "DOLE\n";
        break;
    case 1:
        std::cout << "HORE\n";
        break;
    case 2:
        std::cout << "VPRAVO\n";
        break;
    case -2:
        std::cout << "VLAVO\n";
        break;
    default:
        break;
    }
    op[n->G] = n->operation;
}       

/*
    Testovacia funkcia
*/
void testOutput(int* op, char** state, Node* output, int width, int height) {
    int X_ = 0, Y_ = 0;
    std::cout << "vstup:\n";
    for (int i = 0; i < output->map->height; i++)
    {
        for (int j = 0; j < output->map->width; j++) {
            if (state[i][j] == 0)
            {
                X_ = j;
                Y_ = i;
            }
            printf("%d ", state[i][j]);
        }
        putchar('\n');
    }

    std::cout << "vystup:\n\n\n";

    /*
        cyklus podla operácií rekonštruuje jednotlivé kroky algoritmu
        prièom vypíše aktuálny stav hlavolamu
    */
    for (int i = 0; i <= output->G; i++) {

        switch (op[i])
        {
        case -1:
            std::cout << "\n\nDOLE\n";
            state[Y_][X_] = state[Y_ - 1][X_];
            state[Y_ - 1][X_] = 0;
            Y_--;

            break;
        case 1:
            std::cout << "\n\nHORE\n";
            state[Y_][X_] = state[Y_ + 1][X_];
            state[Y_ + 1][X_] = 0;
            Y_++;
            break;
        case 2:
            std::cout << "\n\nVPRAVO\n";
            state[Y_][X_] = state[Y_][X_ - 1];
            state[Y_][X_ - 1] = 0;
            X_--;
            break;
        case -2:
            std::cout << "\n\nVLAVO\n";
            state[Y_][X_] = state[Y_][X_ + 1];
            state[Y_][X_ + 1] = 0;
            X_++;
            break;

        default:
            break;
        }
        for (int i = 0; i < output->map->height; i++)
        {
            for (int j = 0; j < output->map->width; j++) {

                printf("%d ", state[i][j]);
            }
            putchar('\n');
        }

    }



}

/*
    Funkcia uvo¾ní alokované uzly z hash tabulky map
*/

void freeAll(std::unordered_map<ulli, Node*> map, int height) {
    int a = 0;

    for (auto& it : map) {
        a++;
        for (int i = 0; i < height; i++) {
            delete[]it.second->state[i];
        }
        delete it.second;
    }
}

int main()
{
    int width, height, input, heuristic, flag;
    while (1) {
        std::cout << "Pre nacitanie udajov zo suboru sltac 1\nPre manualne zadavanie vstupnych informacii stlac 2\nPre ukoncenie iny lubovolny znak\n" << std::endl;

        std::unordered_map<ulli, Node*> nodes; // hash tabulka s vytvorenymi uzlami.
        char** arr = NULL; // cielový stav
        char** arra = NULL; // vstupný stav
        Node* n = NULL;


        std::cin >> input;

        if (input == 2)
        {
            flag = 0;
            std::cout << "Zadaj rozmer hlavolamu:\n sirka : ";
            std::cin >> width;
            std::cout << " vyska : ";
            std::cin >> height;
            std::cout << "Zadaj cielovy stav\nJednotlive cisla musia byt oddelene medzerou a v rozsahu 0-14!\n" << std::endl;

            arr = new char* [height];
            for (int i = 0; i < height; i++) {
                arr[i] = new char[width];
                for (int j = 0; j < width; j++) {
                    scanf_s("%d", &arr[i][j]);
                    if (arr[i][j] > 14 || arr[i][j] < 0) {
                        flag = 1;
                    }
                }
            }

            Playground map = Playground(width, height, arr); // inicializácia hlavolamu

            std::cout << "Zadaj pociatocny stav" << std::endl;

            arra = new char* [height];
            for (int i = 0; i < height; i++) {
                arra[i] = new char[width];
                for (int j = 0; j < width; j++) {

                    scanf_s("%d", &arra[i][j]);
                    if (arra[i][j] > 14 || arra[i][j] < 0) {
                        flag = 1;
                    }
                }
                memcpy(arr[i], arra[i], width);
            }
            
            if (flag) {
                std::cout << "Nespravne zadany udaj, opakuj znova!" << std::endl;
                continue;;
            }

            std::cout << "Pre pouzitie heuristiky : Pocet nespravne polozenych policok stlac 1\nPre pouzitie heuristiky : Manhattanska cesta stlac 2\nAk je zadany iny prikaz je pouzita heuristika : Manhattanska cesta" << std::endl;
            std::cin >> heuristic;
            //výber heuristiky
            if (heuristic == 1) {
                map.setHeuristic(Heuristic::heuristic1);
            }
            else {
                map.setHeuristic(Heuristic::heuristic2);
            }

            std::chrono::steady_clock sc;   // èasovaè
            auto start = sc.now();

            nodes = map.findCombination(arra);

            n = map.targetNode;
            auto end = sc.now();        // uknoèenie merania èasu
            auto time_span = static_cast<std::chrono::duration<double>>(end - start);   // zmerá èas, ktorý prebehol poèas vykonávania algoritmu
            std::cout << "Operation took: " << time_span.count() << " seconds !!!\n\n\n";

        }
        else if (input == 1) {
            flag = 0;
            std::string name;

            std::cout << "Zadaj nazov suboru" << std::endl;
            std::cin >> name;

            FILE* inputFile;
            fopen_s(&inputFile, name.c_str(), "r");
            
            if (!inputFile){
                std::cout << "Nepodarilo sa otvort subor!" << std::endl;
                continue;
            }
    

            fscanf_s(inputFile, "%d\n%d", &width, &height);


            arr = new char* [height];
            for (int i = 0; i < height; i++) {
                arr[i] = new char[width];
                for (int j = 0; j < width; j++) {
                    fscanf_s(inputFile, "%d", &arr[i][j]);
                    if (arr[i][j] > 14 || arr[i][j] < 0) {
                        flag = 1;
                    }
                }
            }
          

            Playground map = Playground(width, height, arr); // inicializácia hlavolamu
            std::cout << "Pre pouzitie heuristiky : Pocet nespravne polozenych policok stlac 1\nPre pouzitie heuristiky : Manhattanska cesta stlac 2\nAk je zadany iny prikaz je pouzita heuristika : Manhattanska cesta" << std::endl;
            std::cin >> heuristic;
            // výber heuristiky
            if (heuristic == 1) {
                map.setHeuristic(Heuristic::heuristic1);
            }
            else {
                map.setHeuristic(Heuristic::heuristic2);
            }


            arra = new char* [height];
            for (int i = 0; i < height; i++) {
                arra[i] = new char[width];
                for (int j = 0; j < width; j++) {

                    fscanf_s(inputFile, "%d", &arra[i][j]);
                    if (arra[i][j] > 14 || arra[i][j] < 0) {
                        flag = 1;
                    }
                }
                memcpy(arr[i], arra[i], width);
            }
            if (flag) {
                std::cout << "Nespravne zadany udaj, opakuj znova!" << std::endl;
                continue;
            }

            std::chrono::steady_clock sc;   // èasovaè
            auto start = sc.now();

            nodes = map.findCombination(arra);

            n = map.targetNode;
            auto end = sc.now();       // uknoèenie merania èasu
            auto time_span = static_cast<std::chrono::duration<double>>(end - start);   // zmerá èas, ktorý prebehol poèas vykonávania algoritmu
            std::cout << "Operation took: " << time_span.count() << " seconds !!!\n\n\n";

            if(inputFile)
                fclose(inputFile); 
        }
        else
            break;
        
        int* operators = NULL;
        if (n) {
            operators = new int[n->G + 1];
            printOperators(n, operators); // vypíše operátory
            // testovacia funkcia
            //testOutput(operators, arr, n, width, height);
            delete[] operators;
        }
        else {
            std::cout << "Nebolo mozne sa dopracovat ku koncovemu stavu\n" << std::endl;
        }
        freeAll(nodes, height); // uvolnenie alokovanej pamäti
        
    }
}

