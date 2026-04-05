# Game AI Project


A Digital Arts and Entertainment project on Game Ai, using Unreal Engine.  

---

### Steering Behaviours

The project has basic Steering Behaviours, such as seek, flee, arrive, face, pursue, and evade.  
These can then be combined into Blended Steering: using multiple behaviours at once, 
each with their own weight, and Priority Steering: using different behaviours depending on the situation.
  

### Flocking

These Steering Behaviours can be combined into a Flock, where the bird-oids adjust their behaviour to eachother.
To optimize the performance when the Flock is quite big, flat space partitioning has been implemented.
  

### Pathfinding

The agents can currently pathfind with a simple Breadth-First Search or A-star pathfinding.  
When using a NavMesh, these paths can then be smoothened with an SSFA.