import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class Boomerang implements Runnable
{
	public long simTime;
	public int trafficGenRate;
	public int m; 
	public int n;
	public int buffSize;
	public int pktSize;
	
	// Network information
	public ArrayList<Node> nodes;
	
	// Main simulation thread control
	private volatile boolean running = true;
	
	public Boomerang(long st, int num, int tgr, int mm, int nn, int bs, int ps)
	{
		this.simTime = st;
		this.trafficGenRate = tgr;
		this.m = mm;
		this.n = nn;
		this.buffSize = bs;
		this.pktSize = ps;
		
		// Create all of the nodes
		nodes = new ArrayList<Node>();
		for (int i = 0; i < num; i++)
		{
			nodes.add(new Node(i, this));
		}
	}
	
	public void run()
	{
		long start = System.currentTimeMillis() / 1000;
		long current = System.currentTimeMillis() / 1000;
		
		// Start every node
		System.out.println("Simulation starting.");
		for (Node n : nodes)
		{
			n.start();
		}
		
		// Run the simulation for the specified amount of time
		while ((current - start) < simTime)
		{
			// let everything advance...
			
			// Update the simulation time
			current = System.currentTimeMillis() / 1000;
		}
		
		// Kill all the nodes
		System.out.println("Simulation complete. Killing every node.");
		for (Node n : nodes)
		{
			n.stop();
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
			int trafficGenRate = Integer.parseInt(reader.readLine());
			int m = Integer.parseInt(reader.readLine());
			int n = Integer.parseInt(reader.readLine());
			int buffSize = Integer.parseInt(reader.readLine());
			int pktSize = Integer.parseInt(reader.readLine());
			
			// Create the simulator
			Boomerang boom = new Boomerang(simTime, numNodes, trafficGenRate, m, n, buffSize, pktSize);
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
