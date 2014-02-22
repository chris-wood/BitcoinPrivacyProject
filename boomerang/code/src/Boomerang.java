import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicInteger;

import org.ho.yaml.Yaml;

public class Boomerang implements Runnable
{
	// Parameter collection
	Config config;
	
	// Stats
	//***1. Bandwidth consumption (average amount of data in the network)
	//***2. Number of "computations" done per node (i.e., what's the total cost spent by each node)
	//***3. Total message latency (from start to end of a circuit)
	//***4. Per-hop latency 
	//***5. Node forwarding throughput (messages/s)
	//***6. #retries
	public AtomicInteger numMessages;
	
	// Network information
	public ArrayList<Node> nodes;
	public HashSet<Message> messages;
	public HashSet<Message> messagesToRemove;
	public ArrayList<Message> completedMessages;
	
	// State
	public long spawnWait;
	public boolean startSpawn;
	public long termWait;
	public boolean startTerm;
	
	public Random rng;
	
	// Event collector
	public static BlockingQueue<MessageEvent> events; 
	
	public Boomerang(Config config)
	{	
		this.config = config;
		this.spawnWait = 0L;
		this.startSpawn = true;
		this.termWait = 0L;
		this.startTerm = true;
		
		rng = new Random(config.seed++);
		
		// Stats
		numMessages = new AtomicInteger(0);
		
		// Event collector
		events = new LinkedBlockingQueue<MessageEvent>();
		
		// RNG for random node locations
		Random rng = new Random(config.seed);
		
		// Create all of the nodes
		nodes = new ArrayList<Node>();
		for (int i = 0; i < config.numNodes; i++)
		{
			int x = rng.nextInt(config.gridWidth);
			int y = rng.nextInt(config.gridHeight);
			nodes.add(new Node(new Location(x, y), this, config.seed++));
		}
		
		// Message container
		messages = new HashSet<Message>();
		messagesToRemove = new HashSet<Message>();
		completedMessages = new ArrayList<Message>();
	}
	
	public void computeStats() throws IOException
	{
		// 1. compute all stats
		// 2. generate plot of nodes with message paths
		
//		strict graph {
//		    node0 [pos="1,2!"];
//		    node1 [pos="2,3!"];
//		}
		
		// Create the main adjacency matrix of completed broadcasts
		int[][] completeAM = new int[Node.globalNodeId][Node.globalNodeId];
		for (int i = 0; i < Node.globalNodeId; i++)
		{
			for (int j = 0; j < Node.globalNodeId; j++)
			{
				for (Message m : completedMessages)
				{
					for (int n = 0; n < m.circuit.size() - 1; n++)
					{
						completeAM[m.circuit.get(n).id][m.circuit.get(n+1).id]++; // increase weight on edge between the graph
					}
				}
			}
		}
		
		PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(config.outfileprefix + "_complete.txt")));
		for (int i = 0; i < Node.globalNodeId; i++)
		{
			StringBuilder builder = new StringBuilder();
			for (int j = 0; j < Node.globalNodeId; j++)
			{
				builder.append(completeAM[i][j] + ",");
			}
			System.out.println(builder.substring(0, builder.toString().length() - 1));
			writer.println(builder.substring(0, builder.toString().length() - 1));
		}
		writer.flush();
		writer.close();
		
		// TODO: generate adj matrices of:
		// 1. all transactions
		// 2. complete transactions
		// 2. all chaff
		// 3. complete chaff
	}
	
	public void doEvent()
	{
		// Handle generation
		if (spawnWait > 0)
		{
			spawnWait--;
		}
		else if (startSpawn) // wait time = 0
		{
			startSpawn = false;
			int sleepSeconds = rng.nextInt(config.enterRate) + 1;
			spawnWait = sleepSeconds;
		}
		else
		{
			// Add the new node to the group
			int x = rng.nextInt(config.gridWidth);
			int y = rng.nextInt(config.gridHeight);
			Node newNode = new Node(new Location(x, y), this, config.seed++);
			addNode(newNode);
			System.err.println("Spawning " + newNode + " at time " + Clock.time);
			startSpawn = true;
		}
		
		// Handle termination
		if (termWait > 0)
		{
			termWait--;
		}
		else if (startTerm) // wait time = 0
		{	
			startTerm = false;
			int sleepSeconds = rng.nextInt(config.exitRate) + 1;
			termWait = sleepSeconds;
		}
		else
		{
			removeRandomNode();
			startTerm = true;
		}
	}
	
	public void run()
	{	
		// Run the simulation for the specified amount of time
		Util.disp("Simulation starting...");
		Clock clock = new Clock(); 
		while (clock.time < config.simTime)
		{
			for (Node n : nodes)
			{
				n.doEvent();
			}
			
			for (Message m : messages)
			{
				m.doEvent();
			}
			
			for (Message m : messagesToRemove)
			{
				messages.remove(m);
			}
			messagesToRemove.clear();
			
			doEvent();
			
			// Advance time
			clock.tick();
		}
		
		// Kill all the nodes
		Util.disp("Simulation complete. Killing every node.");
	}
	
	public void finalizeMessage(Message m)
	{
		messagesToRemove.add(m);
		completedMessages.add(m);
	}
	
	public void addMessage(Message m)
	{
		messages.add(m);
	}
	
	public Node getNode(int index)
	{
		return nodes.get(index);
	}
	
	public int getNumberOfNodes()
	{
		int size = nodes.size();
		return size;
	}
	
	public void removeNode(int index)
	{
		nodes.remove(index);
	}
	
	public void removeRandomNode()
	{
		if (getNumberOfNodes() > 0)
		{
			int index = rng.nextInt(getNumberOfNodes());
			Node n = nodes.get(index);
			nodes.remove(index);
			System.err.println("Terminating " + n + " at time " + Clock.time);
		}
	}
	
	public void addNode(Node n)
	{
		nodes.add(n);
	}
	
	public static void main(String[] args)
	{
		if (args.length != 1)
		{
			System.err.println("Usage: java Boomerang config");
			System.exit(-1);
		}
		
		try
		{
			// Parse the config file and run the simulator
			Config config = Yaml.loadType(new File(args[0]), Config.class);
			Util.disp(config.toString());
			
			// Run the simulator
			Boomerang boom = new Boomerang(config);
			boom.run();
			boom.computeStats();
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
		}
		catch (NumberFormatException e)
		{
			e.printStackTrace();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
	}
}
