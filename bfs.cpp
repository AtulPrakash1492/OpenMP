#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstddef>
#include <omp.h>

#include "CycleTimer.h"
#include "bfs.h"
#include "graph.h"

#define ROOT_NODE_ID                    0
#define NOT_VISITED_MARKER              -1
#define BOTTOMUP_NOT_VISITED_MARKER     0
#define PADDING                         16
#define THRESHOLD                       10000000




void vertex_set_clear(vertex_set* list) {
    list->count = 0;
}

void vertex_set_init(vertex_set* list, int count) {
    list->alloc_count = count;
    list->present = (int*)malloc(sizeof(int) * list->alloc_count);
    vertex_set_clear(list);
}







void bottom_up_step(
    graph* g,
    vertex_set* frontier,    
    int* distances,
    int iteration) {
    int local_count = 0;
    int padding[15];
    #pragma omp parallel 
    {
        #pragma omp for reduction(+:local_count)
        for (int i=0; i < g->num_nodes; i++) {                   
            if (frontier->present[i] == BOTTOMUP_NOT_VISITED_MARKER) {
                int start_edge = g->incoming_starts[i];
                int end_edge = (i == g->num_nodes-1)? g->num_edges : g->incoming_starts[i + 1];
                for(int neighbor = start_edge; neighbor < end_edge; neighbor++) {
                    int incoming = g->incoming_edges[neighbor];
                    if(frontier->present[incoming] == iteration) {
                        distances[i] = distances[incoming] + 1;                        
                        local_count ++;
                        frontier->present[i] = iteration + 1;
                        break;
                    }
                }
            }
        }
    }    
    frontier->count = local_count;

}

void bfs_bottom_up(graph* graph, solution* sol)
{

    vertex_set list1;
    
    vertex_set_init(&list1, graph->num_nodes);
    
    int iteration = 1;

    vertex_set* frontier = &list1; 
    
    memset(frontier->present, 0, sizeof(int) * graph->num_nodes);

    frontier->present[frontier->count++] = 1;

    for (int i=0; i<graph->num_nodes; i++)
        sol->distances[i] = 0;
    while (frontier->count != 0) {
        
        frontier->count = 0;

        bottom_up_step(graph, frontier, sol->distances, iteration);

        iteration++;

    }

    
}
void top_down_step(
    graph* g,
    vertex_set* frontier,    
    int* distances,    
    int iteration)
{
    
    int local_count = 0; 
    int padding[15];  
    #pragma omp parallel 
    {
        #pragma omp for reduction(+ : local_count)
        for (int i=0; i < g->num_nodes; i++) {   
            if (frontier->present[i] == iteration) {             
                int start_edge = g->outgoing_starts[i];
                int end_edge = (i == g->num_nodes-1) ? g->num_edges : g->outgoing_starts[i+1];          
                for (int neighbor=start_edge; neighbor<end_edge; neighbor++) {
                    int outgoing = g->outgoing_edges[neighbor];
                    if(frontier->present[outgoing] == BOTTOMUP_NOT_VISITED_MARKER) {                
                        distances[outgoing] = distances[i] + 1;
                        local_count ++;
                        frontier->present[outgoing] = iteration + 1;
                    }
                }
            }
        }
    }
    frontier->count = local_count;
}

void bfs_top_down(graph* graph, solution* sol) {
    

    vertex_set list1;
    
    vertex_set_init(&list1, graph->num_nodes);    

    int iteration = 1;

    vertex_set* frontier = &list1;
            
    memset(frontier->present, 0, sizeof(int) * graph->num_nodes);

    frontier->present[frontier->count++] = 1;        
    sol->distances[ROOT_NODE_ID] = 0;    
        
    while (frontier->count != 0) {

        frontier->count = 0;
        top_down_step(graph, frontier, sol->distances, iteration);
        iteration++;
    }    

}






void bfs_hybrid(graph* graph, solution* sol) {

    vertex_set list1;
    
    vertex_set_init(&list1, graph->num_nodes);
    
    int iteration = 1;

    vertex_set* frontier = &list1;    
    memset(frontier->present, 0, sizeof(int) * graph->num_nodes);

    frontier->present[frontier->count++] = 1;

    sol->distances[ROOT_NODE_ID] = 0;
    
    while (frontier->count != 0) {
        
        if(frontier->count >= THRESHOLD) {
            frontier->count = 0;
            bottom_up_step(graph, frontier, sol->distances, iteration);
        }
        else {
            frontier->count = 0;
            top_down_step(graph, frontier, sol->distances, iteration);
        }

        iteration++;


    }     
}
