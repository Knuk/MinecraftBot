#ifndef GRAPH_H
#define GRAPH_H

#include "node.h"
#include "world.h"
#include <list>
#include <vector>
#include <algorithm>
#include <queue>

class CompareNode
{
public:
    bool operator()(const Node* x, const Node* y) const
    {
        return (x->gScore + x->hScore) > (y->gScore + y->hScore);
    }
};

class Graph
{
public:
    Graph();
    ~Graph();
    std::vector<Direction> findPath(World * world, Position startPosition, Position endPosition);
private:
    Node * findNode(std::vector<Node*> list, Position pos);

};

#endif // GRAPH_H
