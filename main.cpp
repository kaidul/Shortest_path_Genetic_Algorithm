#include <bits/stdc++.h>

using namespace std;

#define Max 100

#define Iteration 100
#define Elitism true
#define tournamentSize 5
#define Generation 50
#define mutationRate 0.015
#define K 5

int N;
vector <int> adj[Max];
int dist[Max][Max];
bool taken[Max * Max];

struct Individual {
    int cost;
    vector<int> chromosome;
    Individual()
        : cost(0) {
    }
    Individual(int a, vector<int> b)
        : cost(a)
        , chromosome(b) {
    }
};

void printIndividual(const Individual &);

void createIndividual(int curr, int dest, Individual *population, int pathLength, int& indx, vector<int>& chromosome, int cost) {
    taken[curr] = true;
    chromosome.push_back(curr);
    if(curr == dest) {
        population[indx] = Individual(cost, chromosome);
        ++indx;
        return;
    }
    if(pathLength == N) return;

    int adjSize = (int)adj[curr].size();
    bool block = true;
    for(int i = 0; i < adjSize; ++i) {
        if(!taken[adj[curr][i]]) {
            block = false;
            break;
        }
    }
    if(block) return;
    int idx, next;
    // you buggy whore!! fuck u off now!
    do {
        idx = rand() % adjSize;
        next = adj[curr][idx];
    } while(taken[next]);
    int pathCost = cost + dist[curr][next];
    createIndividual(next, dest, population, pathLength + 1, indx, chromosome, pathCost);

}

double getFitness(int cost) {
    return (1.0 / (double)cost);
}

void printIndividual(const Individual& obj) {
    int n = (int)obj.chromosome.size();
    for(int i = 0; i < n - 1; ++i) {
        printf("%d -> ", obj.chromosome[i]);
    }
    printf("%d ", obj.chromosome[n - 1]);
    printf("(%d)\n", obj.cost);
}

int getDistance(const Individual& indv) {
    int cost = 0;
    for(int i = 0, n = indv.chromosome.size(); i < n - 1; ++i) {
        cost += dist[ indv.chromosome[i] ][ indv.chromosome[i + 1] ];
    }
    return cost;
}

Individual getFittest(const Individual *Population, int Size) {
    int fittest = 0;
    double best = getFitness(Population[0].cost);
    for(int i = 1; i < Size; ++i) {
        double fitness = getFitness(Population[i].cost);
        if(fitness > best) {
            fittest = i;
            best = fitness;
        }
    }
    return Population[fittest];
}

bool mutateUtil(Individual& indv, int u, int &dest, int pathLength) {
    taken[u] = true;
    indv.chromosome.push_back(u);
    if(u == dest) return true;
    if(pathLength == N) return false;

    int adjs = (int)adj[u].size();
    bool block = true;
    for(int i = 0; i < adjs; ++i) {
        if(!taken[adj[u][i]]) {
            block = false;
            break;
        }
    }
    if(block) return false;
    int randomIdx, v;
    do {
        randomIdx = rand() % adjs;
        v = adj[u][randomIdx];
    } while(taken[v]);
    indv.cost += dist[u][v];
    return mutateUtil(indv, v, dest, pathLength + 1);
}

void mutate(Individual& indv) {
    double mutationProb = ((double) rand() / (RAND_MAX));
    if(mutationProb < mutationRate) {
        int n = (int)indv.chromosome.size();
        int mutationPoint = rand() % n;
        if(mutationPoint == n - 1) return;
        memset(taken, false, sizeof taken);
        int src = indv.chromosome[mutationPoint], dest = indv.chromosome[n - 1];
        Individual newIndv;
        int cost = 0;
        for(int i = 0; i < mutationPoint; ++i) {
            newIndv.chromosome.push_back(indv.chromosome[i]);
            taken[indv.chromosome[i]] = true;
            cost += dist[indv.chromosome[i]][indv.chromosome[i + 1]];
        }
        newIndv.cost = cost;
        bool success = mutateUtil(newIndv, src, dest, mutationPoint + 1);
        if(success) indv = newIndv;
    }
}

Individual crossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    int len1 = parent1.chromosome.size();
    int len2 = parent2.chromosome.size();
    vector< pair<int, int> > crossingSites;
    for(int i = 1; i < len1 - 1; ++i) {
        for(int j = 1; j < len2 - 1; ++j) {
            if(parent1.chromosome[i] == parent2.chromosome[j])
                crossingSites.push_back( make_pair(i, j) );
        }
    }

    if(crossingSites.size() < 1) return parent1.cost >= parent2.cost ? parent1 : parent2;

    pair<int, int> randomCS = crossingSites[rand() % crossingSites.size()];

    for(int i = 0; i < randomCS.first; ++i)
        child.chromosome.push_back(parent1.chromosome[i]);
    for(int i = randomCS.second; i < len2; ++i)
        child.chromosome.push_back(parent2.chromosome[i]);

    child.cost = getDistance(child);

    return child;
}

Individual tournamentSelection(const Individual* pop) {
    Individual tournament[tournamentSize];
    for(int i = 0; i < tournamentSize; ++i) {
        int randomID = rand() % Iteration;
        tournament[i] = pop[randomID];
    }
    return getFittest(tournament, tournamentSize);

}
//main ga process
Individual* evolvePopulation(const Individual* Pop) {
    int elitismOffset = 0;
    // memory leak ? Nop, fixed now!
    Individual *evolvePop = new Individual[Iteration];

    if(Elitism) {
        Individual best = getFittest(Pop, Iteration);
        evolvePop[0] = best;
        elitismOffset = 1;
    }

    for(int i = elitismOffset; i < Iteration; ++i) {
        Individual parent1 = tournamentSelection(Pop);
        Individual parent2 = tournamentSelection(Pop);
        Individual child = crossover(parent1, parent2);
        evolvePop[i] = child;
    }

    delete [] Pop;

    for(int i = elitismOffset; i < Iteration; ++i) {
        mutate(evolvePop[i]);
    }

    return evolvePop;
}

void generatePopulation(int Size, int& source, int& destination, Individual *population) {
    int indx = 0;
    while(indx != Size) {
        vector <int> chromosome;
        memset(taken, false, sizeof taken);
        createIndividual(source, destination, population, 1, indx, chromosome, 0);
    }
}

int compare(const void *vI1, const void *vI2) {
    Individual *I1 = (Individual *)vI1;
    Individual *I2 = (Individual *)vI2;

    return (*I1).cost > (*I2).cost;
}

void k_bestSolution(Individual* pop) {
    qsort(&pop[0], Iteration, sizeof(Individual), compare);
    int k = min(Iteration, K);
    printf("Total Population: %d\n", Iteration);
    printf("%d most optimal solutions:\n", k);
    for(int i = 0; i < k; ++i) {
        printIndividual(pop[i]);
    }
}

int main(void) {
    freopen("input.txt", "r", stdin);
//    freopen("output.txt", "w", stdout);
    int u, v, cost, edge;
    scanf("%d %d", &N, &edge); // N = vertices
    for(int i = 0; i < edge; ++i) {
        scanf("%d %d %d", &u, &v, &cost);

        // adjacency list representation
        adj[u].push_back(v);
        adj[v].push_back(u);

        // adjacency matrix
        dist[u][v] = dist[v][u] = cost;
    }
    int source, destination;
    int idx, trafficPoint;

    while(scanf("%d %d", &source, &destination) == 2) {
        if(!source and !destination) break;

        Individual *population = new Individual[Iteration]; // pointer to an array

        cout << "generating Population .....\n";
        generatePopulation(Iteration, source, destination, population);
        cout << "Population generation success!\n";

        Individual *newPopulation = evolvePopulation(population);

        for(int i = 0; i < Generation; ++i) {
            newPopulation = evolvePopulation(newPopulation);
        }

        k_bestSolution(newPopulation);

        delete [] newPopulation;
    }
    return 0;
}
