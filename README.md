# UCS BFS AStar algorithms

Usage:

	main --ss {state_file.txt} --alg {ucs|bfs|astar} [--h heuristic_file.txt]

Examples of state and heuristic files can be found in the maps folder.

Concrete examples executed from the src folder:

	main --ss ../maps/ai.txt --alg ucs --h ../maps/ai_fail_heuristic.txt 
	main --ss ../maps/ai.txt --alg astar --h ../maps/ai_pass_heuristic.txt
	main --ss ../maps/ai.txt --alg bfs
