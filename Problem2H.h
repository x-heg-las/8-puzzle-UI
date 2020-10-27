
#define UP 1
#define DOWN -1
#define RIGHT 2
#define LEFT -2
#define MOVEMENTS 4


#include <vector>
#include <functional>
#include <queue>
#include <algorithm>
#include <set>
#include<string.h>
#include <unordered_map>
#include <stdio.h>


typedef unsigned long long int ulli;



namespace Hlavolam
{

	class Playground;

	struct Node {

		unsigned char  H; // G- hlbka H - vysledok heuristiky
		unsigned char G;
		char operation; 
		char** state;
		Node* parent;
		Playground* map;

		/*
			Konštruktor, ktorý inicializuje potrebné premenné pre uzol.
		*/
		Node(char** state_, int width, int height, Node* parent_, char operation_, unsigned char G_, Playground* map_) {
			G = G_;
			H = 0;
			operation = operation_;
			map = map_;
			parent = parent_;
			state = new char* [height];
			for (int i = 0; i < height; i++) {
				state[i] = new char[width];
				memcpy(state[i], state_[i], width);
			}
		}

		bool isLast();
		unsigned int score();
	};

	/*
		Funkcia, ktorá je použitá pri porovnávaní cien keï uzol vkládame do fronty std::priority_queue<Node*, std::vector<Node*>, CompareNodes>;
	*/
	struct CompareNodes {
		bool operator()(Node* const& n1, Node* const& n2) {
			
			return (n1->G + n1->H) > (n2->H + n2->G);
		}
	};

	
	using openSet = std::priority_queue<Node*, std::vector<Node*>, CompareNodes>;
	using Heuristics = std::function<unsigned char(Node* current)>;

	/*
		Trieda reprezentujuca hlavolam
	*/
	class Playground {

	public:
		Playground(int width_, int height_, char** targetConfiguration);

		Node* targetNode; // ukazovatel na hladaný stav
		
		void setHeuristic(Heuristics function_);
		std::unordered_map<ulli, Node*> findCombination(char** startNode);
		Heuristics heuristicFunction; // heuristická funkcia
		char width, height; // šírka a výška hlavolamu
	};

	class Heuristic {

	public:
		static unsigned char heuristic1(Node *current); // Poèet zle položených políèok
		static unsigned char heuristic2(Node *current);	// Manhattanská cesta

	};

}