# UCS BFS AStar algorithms

Usage:

	main --ss {state_file.txt} --alg {ucs|bfs|astar} [--h heuristic_file.txt]

Examples of state and heuristic files can be found in the maps folder.
State files have the following format:

	name_of_start_node
	name_of_end_node[ name_of_a_different_end_node ...]
	node1: node1_neighbour1,cost[ node1_neighbour2,cost ...]
	node2: node2_neighbour1,cost[ node2_neighbour2,cost ...]
	...

Similarly, heuristic files:

	node1: heuristic1
	node2: heuristic2
	node3: heuristic3
	...

Names of nodes can be anything as long as they're consistent between the state and the heuristic file.

Concrete examples executed from the src folder:

	main --ss ../maps/ai.txt --alg ucs
	main --ss ../maps/ai.txt --alg astar --h ../maps/ai_pass_heuristic.txt
	main --ss ../maps/ai.txt --alg bfs
