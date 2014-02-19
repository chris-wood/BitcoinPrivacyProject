import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;


public class Node 
{
	// Behavior parameters
	public Boomerang boom;
	public int id;
	public Random rng;
	public Location loc;
	
	// Inner processes
	NodeForwarder forwarder;
	NodeGenerator generator;
	
	// Buff node things
	public BlockingQueue<Message> msgQueue;
	public int msgIndex = 0;
	
	public Node(int id, Location loc, Boomerang boomerang, long seed)
	{
		this.id = id;
		this.boom = boomerang;
		this.loc = loc;
		this.rng = new Random(seed);
		this.msgQueue = new LinkedBlockingQueue<Message>();
	}
	
	public void start()
	{
		this.forwarder = new NodeForwarder(this);
		Thread t1 = new Thread(forwarder);
		t1.start();
		
		this.generator = new NodeGenerator(this);
		Thread t2 = new Thread(generator);
		t2.start();
	}
	
	public void stop()
	{
		forwarder.kill();
		generator.kill();
	}
	
	public void acceptMessage(Message m) throws InterruptedException
	{
		if (m.hops.size() > 1)
		{
			m.arriveTime.add(System.currentTimeMillis());
			msgQueue.put(m);
		}
		else
		{
			// reached the end of the circuit, broadcast the transaction here
			System.err.println(m + " reached the end of the circuit");
		}
	}
	
	class NodeForwarder implements Runnable
	{
		public Node host; 
		
		// Node thread control
		private volatile boolean running = true;
		
		public NodeForwarder(Node host)
		{
			this.host = host;
		}
		
		public void kill()
		{
			try
			{
				msgQueue.put(new EndMessage("PP" + id)); // poison pill
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
			running = false;
		}

		@Override
		public void run() 
		{
			while (running)
			{
				ArrayList<Message> bucket = new ArrayList<Message>();
				for (int i = 0; i < boom.buffSize; i++)
				{
					try
					{
						Message m = msgQueue.take();
						if (!(m instanceof EndMessage))
						{
							bucket.add(m);
						}
						else
						{
							running = false; // redundant
							break;
						}
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						System.exit(-1);
					}
				}
				
				if (running)
				{
//					System.out.println("Node " + id + " mixing and forwarding");
					Collections.shuffle(bucket);
					for (Message m : bucket)
					{
						m.sendTime.add(System.currentTimeMillis());
						Thread mThread = new Thread(m);
						mThread.run();
					}
				}
			}
		}
	}
	
	class NodeGenerator implements Runnable
	{
		private Node host;
		
		// Node thread control
		private volatile boolean running = true;
		
		public void kill()
		{
			running = false;
		}
		
		public NodeGenerator(Node host)
		{
			this.host = host;
		}
		
		@Override
		public void run()
		{
			while (running)
			{		
				try
				{
					double sleep = rng.nextDouble() * boom.trafficGenRate;
					sleep = sleep < 0 ? (sleep * -1) % boom.trafficGenRate : sleep;
					Thread.sleep((long)sleep * 1000);
					
					// Build the m circuits of length n each
					ArrayList<Message> messages = new ArrayList<Message>();
					for (int m = 0; m < boom.m; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						for (int n = 0; n < boom.n; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt(boom.nodes.size());
							while (seen.contains(nIndex) == true)
							{
								nIndex = rng.nextInt(boom.nodes.size());
							}
							
							circuit.add(boom.nodes.get(nIndex));
							seen.add(nIndex);
						}
						
						// New message to send
						Message newMsg = new Message(id + "-" + msgIndex++);
						newMsg.setHops(circuit);
						messages.add(newMsg);
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
//						System.err.println(m.toString() + ": " + m.hops);
//						Node nextHop = m.hops.remove(0);
//						nextHop.acceptMessage(m);
						m.hops.add(0, host);
						m.sendTime.add(System.currentTimeMillis());
						Thread mThread = new Thread(m);
						mThread.run();
					}
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
					running = false;
				}
			}
		}
	}
	
	@Override
	public String toString()
	{
		return "Node-" + id;
	}

}
