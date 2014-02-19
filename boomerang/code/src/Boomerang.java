import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Random;

public class Boomerang implements Runnable
{
	// Parameters
	public long simTime;
	public int enterRate;
	public int exitRate;
	public int trafficGenRate;
	public int gridWidth;
	public int gridHeight;
	public int m; 
	public int n;
	public int buffSize;
	public int mixDelay;
	public int pktSize;
	public long seed;
	
	// Node maintenance threads
	public BoomerangNodeGenerator generator;
	public BoomerangNodeTerminator terminator;
	
	// Network information
	public ArrayList<Node> nodes;
	
	public Boomerang(long seed, long st, int num, int enterRate, int exitRate, int height, int width, int tgr, int mm, int nn, int bs, int md, int ps)
	{
		this.seed = seed;
		this.simTime = st;
		this.enterRate = enterRate;
		this.exitRate = exitRate;
		this.trafficGenRate = tgr;
		this.gridHeight = height;
		this.gridWidth = width;
		this.m = mm;
		this.n = nn;
		this.buffSize = bs;
		this.mixDelay = md;
		this.pktSize = ps;
		
		// RNG for random node locations
		Random rng = new Random(seed);
		
		// Create all of the nodes
		nodes = new ArrayList<Node>();
		for (int i = 0; i < num; i++)
		{
			int x = rng.nextInt(gridWidth);
			int y = rng.nextInt(gridHeight);
			nodes.add(new Node(new Location(x, y), this, seed++));
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
		if (enterRate > 0)
		{
			genThread.start();
		}
		Thread termThread = new Thread(terminator);
		if (exitRate > 0)
		{
			termThread.start();
		}
		
		// Run the simulation for the specified amount of time
		while ((current - start) < simTime)
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
			Random rng = new Random(seed++);
			if (getNumberOfNodes() > 0)
			{
				nodes.remove(rng.nextInt(getNumberOfNodes()));
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
			Random rng = new Random(boom.seed++);
			while (running)
			{
				try
				{
					// Sleep
					int sleepSeconds = rng.nextInt(boom.enterRate) + 1;
					Thread.sleep(sleepSeconds * 1000);
					
					// Add the new node to the group
					int x = rng.nextInt(gridWidth);
					int y = rng.nextInt(gridHeight);
					if (running)
					{
						Node newNode = new Node(new Location(x, y), boom, seed++);
						boom.addNode(newNode);
						newNode.start();
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
			Random rng = new Random(boom.seed++);
			while (running)
			{
				try
				{
					// Sleep
					int sleepSeconds = rng.nextInt(boom.exitRate) + 1;
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
			// Parse the config file
			BufferedReader reader = new BufferedReader(new FileReader(args[0]));
			long simTime = Long.parseLong(reader.readLine());
			int numNodes = Integer.parseInt(reader.readLine());
			int enterRate = Integer.parseInt(reader.readLine());
			int exitRate = Integer.parseInt(reader.readLine());
			int gridHeight = Integer.parseInt(reader.readLine());
			int gridWidth = Integer.parseInt(reader.readLine());
			int trafficGenRate = Integer.parseInt(reader.readLine());
			int m = Integer.parseInt(reader.readLine());
			int n = Integer.parseInt(reader.readLine());
			int buffSize = Integer.parseInt(reader.readLine());
			int mixDelay = Integer.parseInt(reader.readLine());
			int pktSize = Integer.parseInt(reader.readLine());
			long seed = Long.parseLong(reader.readLine());
			reader.close();
			
			// Print out simulation parameters 
			System.out.println("Starting simulation with parameters:");
			System.out.println("  Time: " + simTime);
			System.out.println("  Nodes: " + numNodes);
			System.out.println("  Traffic generation rate: " + trafficGenRate);
			System.out.println("  Circuit width: " + m);
			System.out.println("  Circuit length: " + n);
			System.out.println("  Mix buffer size: " + buffSize);
			System.out.println("  Mix delay: " + mixDelay);
			System.out.println("  Mix message size: " + pktSize);
			System.out.println();
			
			// Run the simulator
			Boomerang boom = new Boomerang(seed, simTime, numNodes, enterRate, exitRate, gridHeight, gridWidth, trafficGenRate, m, n, buffSize, mixDelay, pktSize);
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
