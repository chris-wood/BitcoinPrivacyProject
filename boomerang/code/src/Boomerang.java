import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Random;
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
	
	// Node maintenance threads
	public BoomerangNodeGenerator generator;
	public BoomerangNodeTerminator terminator;
	
	// Network information
	public ArrayList<Node> nodes;
	
	public Boomerang(Config config)
	{	
		this.config = config;
		
		// Stats
		numMessages = new AtomicInteger(0);
		
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
		
		// Create node maintenance threads
		generator = new BoomerangNodeGenerator(this);
		terminator = new BoomerangNodeTerminator(this);
	}
	
	public void run()
	{
		long start = System.currentTimeMillis() / 1000;
		long current = System.currentTimeMillis() / 1000;
		
		// Start every node
		System.out.println("Simulation starting...");
		for (Node n : nodes)
		{
			n.start();
		}
		
		// Start maintenance threads
		Thread genThread = new Thread(generator);
		if (config.enterRate > 0)
		{
			genThread.start();
		}
		Thread termThread = new Thread(terminator);
		if (config.exitRate > 0)
		{
			termThread.start();
		}
		
		// Run the simulation for the specified amount of time
		while ((current - start) < config.simTime)
		{
			try
			{
				Thread.sleep(1000); // sleep for a second
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
			current = System.currentTimeMillis() / 1000;
		}
		
		// Kill the maintenantce threads
		generator.kill();
		terminator.kill();
		try
		{
			genThread.join();
			termThread.join();
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}
		
		// Kill all the nodes
		System.out.println("Simulation complete. Killing every node.");
		for (Node n : nodes)
		{
			n.stop();
		}
	}
	
	public int getNumberOfNodes()
	{
		int size = nodes.size();
		synchronized(nodes)
		{
			size = nodes.size();
			nodes.notifyAll();
		}
		return size;
	}
	
	public Node getNode(int index)
	{
		Node n;
		synchronized(nodes)
		{
			n = nodes.get(index);
			nodes.notifyAll();
		}
		return n;
	}
	
	public void removeNode(int index)
	{
		synchronized(nodes)
		{
			nodes.remove(index);
			nodes.notifyAll();
		}
	}
	
	public void removeRandomNode()
	{
		synchronized(nodes)
		{
			Random rng = new Random(config.seed++);
			if (getNumberOfNodes() > 0)
			{
				int index = rng.nextInt(getNumberOfNodes());
				Node n = nodes.get(index);
				nodes.remove(index);
				System.err.println("Terminating " + n);
			}
			nodes.notifyAll();
		}
	}
	
	public void addNode(Node n)
	{
		synchronized(nodes)
		{
			nodes.add(n);
			nodes.notifyAll();
		}
	}
	
	class BoomerangNodeGenerator implements Runnable
	{
		// Main simulation thread control
		private volatile boolean running = true;
		
		private Boomerang boom;
		
		public BoomerangNodeGenerator(Boomerang boom)
		{
			this.boom = boom;
		}
		
		public void kill()
		{
			running = false;
		}

		@Override
		public void run()
		{
			Random rng = new Random(boom.config.seed++);
			while (running)
			{
				try
				{
					// Sleep
					int sleepSeconds = rng.nextInt(boom.config.enterRate) + 1;
					Thread.sleep(sleepSeconds * 1000);
					
					// Add the new node to the group
					int x = rng.nextInt(config.gridWidth);
					int y = rng.nextInt(config.gridHeight);
					if (running)
					{
						Node newNode = new Node(new Location(x, y), boom, config.seed++);
						boom.addNode(newNode);
						newNode.start();
						System.err.println("Spawning " + newNode);
					}
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
		}
	}
	
	class BoomerangNodeTerminator implements Runnable
	{
		// Main simulation thread control
		private volatile boolean running = true;
		
		private Boomerang boom;
		
		public BoomerangNodeTerminator(Boomerang boom)
		{
			this.boom = boom;
		}
		
		public void kill()
		{
			running = false;
		}

		@Override
		public void run()
		{
			Random rng = new Random(boom.config.seed++);
			while (running)
			{
				try
				{
					// Sleep
					int sleepSeconds = rng.nextInt(boom.config.exitRate) + 1;
					Thread.sleep(sleepSeconds * 1000);
					
					// Remove a random node
					boom.removeRandomNode();
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
				}
			}
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
			System.out.println(config);
			
			// Run the simulator
			Boomerang boom = new Boomerang(config);
			boom.run();
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
