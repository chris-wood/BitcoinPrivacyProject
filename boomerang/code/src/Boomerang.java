import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashSet;
import java.security.SecureRandom;

import org.ho.yaml.Yaml;

public class Boomerang implements Runnable
{
	public static final long FLOOD_TIME_THRESHOLD = 5000; //30000;
	
	// Parameter collection
	Config config;
	
	// Stats
	//***1. Bandwidth consumption (average amount of data in the network)
	//***2. Number of "computations" done per node (i.e., what's the total cost spent by each node)
	//***3. Total message latency (from start to end of a circuit)
	//***4. Per-hop latency 
	//***5. Node forwarding throughput (messages/s)
	//***6. #retries
	public int numMessages;
	public int numMessagesCompleted;
	public int numChaffGenerated;
	public int numChaffCompleted;
	public int numTxGenerated;
	public int numTxCompleted;
	
	public ArrayList<Message> completedMessages;
	public ArrayList<Message> completedChaff;
	public ArrayList<Message> completedTx;
	public ArrayList<Message> startedChaff;
	public ArrayList<Message> startedTx;
	
	// Network information
	public HashSet<Node> nodeHistory;
	public ArrayList<Node> nodes;
	public HashSet<Message> messages;
	public HashSet<Message> messagesToRemove;
	
	// State
	public long spawnWait;
	public boolean startSpawn;
	public long termWait;
	public boolean startTerm;
	
	public SecureRandom rng; 
	
	public Boomerang(Config config)
	{	
		this.config = config;
		this.spawnWait = 0L;
		this.startSpawn = true;
		this.termWait = 0L;
		this.startTerm = true;
		
		rng = new SecureRandom();
		
		// Stats
		numMessages = 0;
		numMessagesCompleted = 0;
		numChaffGenerated = 0;
		numChaffCompleted = 0;
		numTxGenerated = 0;
		numTxCompleted = 0;
		
		// RNG for random node locations
		SecureRandom rng = new SecureRandom();
		
		// Create all of the nodes
		nodes = new ArrayList<Node>();
		nodeHistory = new HashSet<Node>();
		for (int i = 0; i < config.numNodes; i++)
		{
			int x = rng.nextInt(config.gridWidth);
			int y = rng.nextInt(config.gridHeight);
			Node n = new Node(new Location(x, y), this, config.seed++);
			nodes.add(n);
			nodeHistory.add(n);
		}
		
		// Message container
		messages = new HashSet<Message>();
		messagesToRemove = new HashSet<Message>();
		
		completedMessages = new ArrayList<Message>();
		completedChaff = new ArrayList<Message>();
		startedChaff = new ArrayList<Message>();
		completedTx = new ArrayList<Message>();
		startedTx = new ArrayList<Message>();
	}
	
	public void computeStats() throws IOException
	{
		// 1. compute all stats
		long totalLatency = 0L;
		for (Message m : completedMessages)
		{
			totalLatency += m.latency; 
		}
		double avgLatency = (double)totalLatency / completedMessages.size();
		
		int totalChaffComputations = 0;
		int totalTxComputations = 0;
		int totalForwarded = 0;
		int totalRetries = 0;
		for (Node n : nodeHistory)
		{
			totalChaffComputations += n.numChaffEncodings;
			totalTxComputations += n.numTxEncodings;
			totalForwarded += n.numForwarded;
			totalRetries += n.numRetries;
		}
		double avgChaffComputations = (double)totalChaffComputations / nodeHistory.size();
		double avgTxComputations = (double)totalTxComputations / nodeHistory.size();
		double avgForwarded = (double)totalForwarded / nodeHistory.size();
		double avgRetries  = (double)totalRetries / nodeHistory.size();
	
		Util.error("Writing general stats: " + config.path + "/" + config.outfileprefix + "_stats.txt");
		PrintWriter statsWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_stats.txt")));
		statsWriter.println(totalLatency + "," + totalChaffComputations + "," 
		+ totalTxComputations + "," + totalForwarded + "," + totalRetries + "," 
				+ avgLatency + "," + avgChaffComputations + "," + avgTxComputations 
				+ "," + avgForwarded + "," + avgRetries);
		statsWriter.flush();
		statsWriter.close();
		
		// Only generate matrices if specified to do so
		if (config.genMatrices)
		{
			Util.error("Computing adjacency matrices");
			Util.error("Total number of nodes: " + Node.globalNodeId);
			Util.error("Completed messages: " + completedMessages.size());
			Util.error("Completed chaff: " + completedChaff.size());
			Util.error("Completed TXs: " + completedTx.size());
			Util.error("Started chaff: " + startedChaff.size());
			Util.error("Started TXs: " + startedTx.size());
			
			// Create the main adjacency matrix of completed broadcasts
			int[][] completeAM = new int[Node.globalNodeId][Node.globalNodeId];
			int[][] completeChaffAM = new int[Node.globalNodeId][Node.globalNodeId];
			int[][] completeTxAM = new int[Node.globalNodeId][Node.globalNodeId];
			int[][] startedChaffAM = new int[Node.globalNodeId][Node.globalNodeId];
			int[][] startedTxAM = new int[Node.globalNodeId][Node.globalNodeId];
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
					
					for (Message m : completedChaff)
					{
						for (int n = 0; n < m.circuit.size() - 1; n++)
						{
							completeChaffAM[m.circuit.get(n).id][m.circuit.get(n+1).id]++; // increase weight on edge between the graph
						}
					}
					
					for (Message m : completedTx)
					{
						for (int n = 0; n < m.circuit.size() - 1; n++)
						{
							completeTxAM[m.circuit.get(n).id][m.circuit.get(n+1).id]++; // increase weight on edge between the graph
						}
					}
					
					for (Message m : startedChaff)
					{
						for (int n = 0; n < m.circuit.size() - 1; n++)
						{
							startedChaffAM[m.circuit.get(n).id][m.circuit.get(n+1).id]++; // increase weight on edge between the graph
						}
					}
					
					for (Message m : startedTx)
					{
						for (int n = 0; n < m.circuit.size() - 1; n++)
						{
							startedTxAM[m.circuit.get(n).id][m.circuit.get(n+1).id]++; // increase weight on edge between the graph
						}
					}
				}
			}
			
			Util.error("Writing adjacency matrices to files");
			
			PrintWriter completeWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_complete.txt")));
			PrintWriter completeCharrWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_completechaff.txt")));
			PrintWriter completeTxWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_completetx.txt")));
			PrintWriter startedChaffWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_startchaff.txt")));
			PrintWriter startedTxWriter = new PrintWriter(new BufferedWriter(new FileWriter(config.path + "/" + config.outfileprefix + "_starttx.txt")));
			for (int i = 0; i < Node.globalNodeId; i++)
			{
				StringBuilder completeBuilder = new StringBuilder();
				StringBuilder startedChaffBuilder = new StringBuilder();
				StringBuilder startedTxBuilder = new StringBuilder();
				StringBuilder completeChaffBuilder = new StringBuilder();
				StringBuilder completeTxBuilder = new StringBuilder();
				for (int j = 0; j < Node.globalNodeId; j++)
				{
					completeBuilder.append(completeAM[i][j] + ",");
					startedChaffBuilder.append(startedChaffAM[i][j] + ",");
					startedTxBuilder.append(startedTxAM[i][j] + ",");
					completeChaffBuilder.append(completeChaffAM[i][j] + ",");
					completeTxBuilder.append(completeTxAM[i][j] + ",");
					
				}
				System.out.println(completeBuilder.substring(0, completeBuilder.toString().length() - 1));
				
				completeWriter.println(completeBuilder.substring(0, completeBuilder.toString().length() - 1));
				completeCharrWriter.println(completeChaffBuilder.substring(0, completeChaffBuilder.toString().length() - 1));
				completeTxWriter.println(completeTxBuilder.substring(0, completeTxBuilder.toString().length() - 1));
				startedChaffWriter.println(startedChaffBuilder.substring(0, startedChaffBuilder.toString().length() - 1));
				startedTxWriter.println(startedTxBuilder.substring(0, startedTxBuilder.toString().length() - 1));
			}
			completeWriter.flush();
			completeWriter.close();
			completeCharrWriter.flush();
			completeCharrWriter.close();
			completeTxWriter.flush();
			completeTxWriter.close();
			startedChaffWriter.flush();
			startedChaffWriter.close();
			startedTxWriter.flush();
			startedTxWriter.close();
		}
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
			nodes.add(newNode);
			nodeHistory.add(newNode);
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
	
	public HashSet<Node> randomNodeSubset(int size)
	{
		HashSet<Node> subset = new HashSet<Node>();
		SecureRandom rng = new SecureRandom();
		for (int i = 0; i < size; i++)
		{
			int index = rng.nextInt(nodes.size());
			while (subset.contains(nodes.get(index)) == false)
			{
				subset.add(nodes.get(index));
			}
		}
		return subset;
	}
	
	public void run()
	{	
		// Run the simulation for the specified amount of time
		Util.error("Simulation starting...");
		
		// Initialize everyone's address book with some set of valid nodes
		for (Node n : nodes)
		{
			for (Node nn : randomNodeSubset(config.initialAddressSize))
			{
				n.addressBook.addValidNode(nn);
			}
		}
		
		long start = 0L;
		long end = 0L;
		
		Clock clock = new Clock();
		while (clock.time < config.simTime)
		{	
			start = System.currentTimeMillis();
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
			
			end = System.currentTimeMillis();
			if ((end - start) > FLOOD_TIME_THRESHOLD)
			{
				Util.error("Network flooding detected.");
				break;
			}
			
			// Advance time
			clock.tick();
		}
		
		// Kill all the nodes
		Util.error("Simulation complete. Killing every node.");
	}
	
	public void finalizeMessage(Message m)
	{	
		messagesToRemove.add(m);
		completedMessages.add(m);
		switch (m.type)
		{
			case TX:
				completedTx.add(m);
				startedTx.remove(m);
				break;
			case COVER:
				completedChaff.add(m);
				startedChaff.remove(m);
				break;
		}
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
			Util.error(config.toString());
			
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
