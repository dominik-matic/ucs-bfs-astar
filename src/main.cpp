#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <list>

using namespace std;


/**
 * Globalne varijable
*/
string startState;
set<string> endStates;
map<string, vector<pair<string, double>>> transitions;
map<string, double> heuristic;
int runningAlgorithm;

/**
 * deklaracija nekih funkcija koje ću definirati nakon maina radi preglednosti
*/
void loadStateSpace(string &ss);
void loadHeuristic(string &h);

/**
 * Struktura Node koja predstavlja jedan čvor
*/
struct Node {
	string name;
	double cost;
	double hCost;
	Node* prev;

	Node(const string name,
		const double cost = 0,
		const double hCost = 0,
		Node* prev = nullptr)
	: name(name), cost(cost), hCost(hCost), prev(prev) {}
};


/**
 * Komparator za set visited (closed)
*/
struct NodeSetComparator {
	inline bool operator()(const Node* n1, const Node* n2) const {
		return n1->name < n2->name;
	} 
};

/**
 * Komparator za multiset toVisit (open)
*/
struct NodeMultiSetComparator {
	inline bool operator()(const Node* n1, const Node* n2) const {
		switch(runningAlgorithm) {
			case 0: // bfs
				return false;
			case 1: // ucs
				if(n1->cost == n2->cost) {
					return (n1->name < n2->name);
				}
			return (n1->cost < n2->cost);
			default: // astar
				if((n1->cost + n1->hCost) == (n2->cost + n2->hCost)) {
					return (n1->name < n2->name);
				}
				return ((n1->cost + n1->hCost) < (n2->cost + n2->hCost));
		}
	}
};

/**
 * typedefovi za open i closed skupove, meni se zovu visited i toVisit
*/
typedef set<Node*, NodeSetComparator> NodeSet;
typedef multiset<Node*, NodeMultiSetComparator> NodeMultiset;




bool goal(Node* n) {
	auto it = endStates.find(n->name);
	return (it != endStates.end());
}

void calculatePath(Node *n, list<string> &path, int &pathLength) {
	path.push_front(n->name);
	++pathLength;
	if(n->prev != nullptr) {
		calculatePath(n->prev, path, pathLength);
	}
}



bool BFS_CMP(const pair<string, double> &elem1, const pair<string, double> &elem2) {
	return (elem1.first < elem2.first);
}

void expandAndSortBFS(Node* n, NodeSet &visited, NodeMultiset &toVisit) {
	vector<pair<string, double>> rawVect = transitions[n->name];
	sort(rawVect.begin(), rawVect.end(), BFS_CMP);
	for(size_t i = 0; i < rawVect.size(); ++i) {
		Node tmpNode = Node(rawVect[i].first);
		if((visited.find(&tmpNode)) == visited.end()) {
			Node* newNode = new Node {rawVect[i].first, n->cost + rawVect[i].second, 0, n};
			toVisit.insert(newNode);
		}
	}
}

void expandAndSortUCS(Node* n, NodeSet &visited, NodeMultiset &toVisit) {
	vector<pair<string, double>> rawVect = transitions[n->name];
	for(size_t i = 0; i < rawVect.size(); ++i) {
		Node tmpNode = Node(rawVect[i].first);
		if((visited.find(&tmpNode)) == visited.end()) {
			Node *newNode = new Node {rawVect[i].first, n->cost + rawVect[i].second, 0, n};
			toVisit.insert(newNode);
		}
	}
}


// struktura u koju spremam cvorove je nepogodna za pretrazivanje po imenu,
// zato imam ovu drugu za brzu provjeru u A* algoritmu
map<string, Node*> open;

void expandAndSortAStar(Node* n, NodeSet &visited, NodeMultiset &toVisit) {
	vector<pair<string, double>> rawVect = transitions[n->name];
	
	for(auto transition : rawVect) {
		Node tmpNode(transition.first, n->cost + transition.second);
		
		auto it1 = visited.find(&tmpNode);
		if(it1 != visited.end()) {
			if((*it1)->cost < tmpNode.cost) {
				continue;
			} else {
				visited.erase(it1);
			}
		}

		auto it2 = open.find(tmpNode.name);
		if(it2 != open.end()) {
			if(it2->second->cost < tmpNode.cost) {
				continue;
			} else {
				toVisit.erase(it2->second);
				open.erase(it2);
			}
		}

		Node *newNode = new Node {transition.first, n->cost + transition.second, heuristic[transition.first], n};
		toVisit.insert(newNode);
		open.insert(make_pair(newNode->name, newNode));
	}	
}

double startAlgorithm(void (*expandAndSort)(Node*, NodeSet&, NodeMultiset&), bool printResults = true) {
	NodeSet visited;
	NodeMultiset toVisit;
	bool found = false;

	Node* startingNode = new Node { startState, 0, heuristic[startState], nullptr };
	Node* goalNode;

	toVisit.insert(startingNode);
	while(toVisit.size() > 0) {
		Node *currentNode = *(toVisit.begin());
		toVisit.erase(toVisit.begin());
		visited.insert(currentNode);
		if(goal(currentNode)) {
			goalNode = currentNode;
			found = true;
			break;
		}
		expandAndSort(currentNode, visited, toVisit);
	}

	if(printResults) {
		cout << "[FOUND_SOLUTION]: ";
		if(!found) {
			cout << "no" << endl;
			exit(-1);
		}
		cout << "yes" << endl;

		int pathLength = 0;
		list<string> path;
		calculatePath(goalNode, path, pathLength);
		cout << "[STATES_VISITED]: " << visited.size() << endl;
		cout << "[PATH_LENGTH]: " << pathLength << endl;
		printf("[TOTAL_COST]: %.1lf\n", goalNode->cost);
		cout << "[PATH]: ";
		for(auto elem : path) {
			if(elem != path.front()) {
				cout << " => ";
			}
			cout << elem;
		}
		cout << endl;
	}
	
	if(found) {
		return goalNode->cost;
	}
	cout << "reutnnignsads\n";
	return 0;
}

void checkConst() {
	bool isConsistent = true;

	for(pair<string, double> h : heuristic) {
		vector<pair<string, double>> neighbours = transitions[h.first];
		for(pair<string, double> neighbour : neighbours) {
			cout << "[CONDITION]: ";
			double neighbourHeuristic = heuristic[neighbour.first];
			if(h.second <= neighbourHeuristic + neighbour.second) {
				cout << "[OK] ";
			} else {
				cout << "[ERR] ";
				isConsistent = false;
			}
			cout << "h(" << h.first << ") <= h(" << neighbour.first << ") + c: ";
			printf("%.1lf <= %.1lf + %.1lf\n", h.second, neighbourHeuristic, neighbour.second);
		}
	}

	cout << "[CONCLUSION]: Heuristic is ";
	if(!isConsistent) {
		cout << "not ";
	}
	cout << "consistent." << endl;;

}

void checkOpt() {
	bool isOptimistic = true;
	for(pair<string, double> h : heuristic) {
		startState = h.first;
		double realCost = startAlgorithm(expandAndSortUCS, false);
		cout << "[CONDITION]: ";
		if(h.second <= realCost) {
			cout << "[OK] ";
		} else {
			cout << "[ERR] ";
			isOptimistic = false;
		}
		cout << "h(" << h.first << ") <= h*: ";
		printf("%.1lf <= %.1lf\n", h.second, realCost);
	}

	cout << "[CONCLUSION]: Heuristic is ";
	if(!isOptimistic) {
		cout << "not ";
	}
	cout << "optimistic." << endl;;

}


int main(int argc, char *argv[]) {
	string algorithm = "";
	string h = "";
	string ss;
	bool checkOptimistic = false;
	bool checkConsistent = false;

	for(int i = 1; i < argc; ++i) {
		string arg = argv[i];

		if(arg == "--alg") {
			string alg = argv[i + 1];
			if(alg != "bfs" &&
				alg != "ucs" &&
				alg != "astar") {
				cerr << "Invalid algorithm: " << argv[i + 1] << endl;
				exit(-1);
			} else {
				algorithm = alg;
				++i;
			}
		} else if(arg == "--ss") {
			ss = argv[++i];
		} else if(arg == "--h") {
			h = argv[++i];
		} else if(arg == "--check-optimistic") {
			checkOptimistic = true;
		} else if(arg == "--check-consistent") {
			checkConsistent = true;
		} else {
			cerr << "Unknown argument: " << arg << endl;
		}
	}

	loadStateSpace(ss);
	loadHeuristic(h);

	if(checkConsistent) {
		cout << "# HEURISTIC-CONSISTENT " << h << endl;
		checkConst();
	} else if(checkOptimistic) {
		cout << "# HEURISTIC-OPTIMISTIC " << h << endl;
		runningAlgorithm = 1;
		checkOpt();
	} else if(algorithm == "bfs") {
		cout << "# BFS " << endl;
		runningAlgorithm = 0;
		startAlgorithm(expandAndSortBFS);
	} else if(algorithm == "ucs") {
		cout << "# UCS " << endl;
		runningAlgorithm = 1;
		startAlgorithm(expandAndSortUCS);
	} else if(algorithm == "astar") {
		cout << "# A-STAR " << h << endl;
		runningAlgorithm = 2;
		startAlgorithm(expandAndSortAStar);
	} else {
		cerr << "Something went wrong :/" << endl;
	}

	return 0;
}

/**
 * Učitava stanja i prijelaze iz datoteke dane argumentom --ss
*/
void loadStateSpace(string &ss) {
	
	fstream file;
	file.open(ss, ios::in);
	if(!file.is_open()) {
		cerr << "Could not open file: " << ss << endl;
		exit(-1);
	}

	string line;

	// read startState
	while(getline(file, line)) {
		if(line[0] == '#') continue;
		startState = line;
		break;
	}

	// read endStates
	while(getline(file, line)) {
		if(line[0] == '#') continue;
		size_t index = 0;
		string token;
		while((index = line.find(' ')) != string::npos) {
			token = line.substr(0, index);
			endStates.insert(token);
			line.erase(0, index + 1);
		}
		endStates.insert(line);
		break;
	}

	// read transitions
	while(getline(file, line)) {
		if(line[0] == '#') continue;
		size_t index = 0;
		string token;
		vector<string> parsedLine;
		while((index = line.find(' ')) != string::npos) {
			token = line.substr(0, index);
			parsedLine.push_back(token);
			line.erase(0, index + 1);
		}
		parsedLine.push_back(line);

		string currentState;
		vector<pair<string, double>> stateCostVect;

		currentState = parsedLine[0].substr(0, parsedLine[0].size() - 1);
		for(size_t i = 1; i < parsedLine.size(); ++i) {
			size_t pos = parsedLine[i].find(',');
			string nextState = parsedLine[i].substr(0, pos);
			double cost = stod(parsedLine[i].substr(pos + 1, parsedLine[i].size()));
			stateCostVect.push_back(make_pair(nextState, cost));
		}

		transitions.insert(make_pair(currentState, stateCostVect));
	}

	file.close();
}


/**
 * Učitava heuristiku iz datoteke dane argumentom --h
*/
void loadHeuristic(string &h) {
	if(h == "") { return; }

	fstream file;
	file.open(h, ios::in);
	if(!file.is_open()) {
		cerr << "Could not open file: " << h << endl;
		exit(-1);
	}

	string line;

	// read heuristic
	while(getline(file, line)) {
		if(line[0] == '#') continue;
		size_t pos = line.find(' ');
		string stateName = line.substr(0, pos - 1);
		double cost = stod(line.substr(pos + 1, line.size()));
		heuristic.insert(make_pair(stateName, cost));
	}

	file.close();
}


