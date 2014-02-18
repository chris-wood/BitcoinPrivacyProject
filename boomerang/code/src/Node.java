import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.LinkedBlockingQueue;


public class Node 
{
	// Behavior parameters
	public Boomerang boom;
	public int id;
	public Random rng;
	
	// Inner processes
	NodeForwarder forwarder;
	NodeGenerator generator;
	
	// Buff node things
	public BlockingQueue<Message> msgQueue;
	public int msgIndex = 0;
	
	public Node(int id, Boomerang boomerang)
	{
		this.id = id;
		this.boom = boomerang;
		this.rng = new Random(System.currentTimeMillis());
		this.msgQueue = new LinkedBlockingQueue<Message>();
		this.forwarder = new NodeForwarder();
		this.generator = new NodeGenerator();
	}
	
	public void start()
	{
		Thread t1 = new Thread(forwarder);
		t1.start();
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
		msgQueue.put(m);
	}
	
	class NodeForwarder implements Runnable
	{
		// Node thread control
		private volatile boolean running = true;
		
		public void kill()
		{
			running = false;
		}

		@Override
		public void run()
		{
			while (running)
			{
				ArrayList<Message> bucket = new ArrayList<Message>();
				System.out.println("Node " + id + " waiting for messages");
				for (int i = 0; i < boom.buffSize; i++)
				{
					try
					{
						bucket.add(msgQueue.take());
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
						System.exit(-1);
					}
				}
				System.out.println("Node " + id + " mixing and forwarding");
			}
		}
	}
	
	class NodeGenerator implements Runnable
	{
		// Node thread control
		private volatile boolean running = true;
		
		public void kill()
		{
			running = false;
		}
		
		@Override
		public void run()
		{
			while (running)
			{		
				try
				{
					int sleep = rng.nextInt() % boom.trafficGenRate;
					sleep = sleep < 0 ? (sleep * -1) % boom.trafficGenRate : sleep;
					Thread.sleep(sleep * 1000);
					
					// Build the m circuits of length n each
					ArrayList<Message> messages = new ArrayList<Message>();
					for (int m = 0; m < boom.m; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						for (int n = 0; n < boom.n; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt() % boom.nodes.size();
							while (seen.contains(nIndex) == false)
							{
								nIndex = rng.nextInt() % boom.nodes.size();
							}
							
							circuit.add(boom.nodes.get(nIndex));
							seen.add(nIndex);
						}
						
						// New message to send
						Message newMsg = new Message(id + "-" + msgIndex++);
						newMsg.setHops(circuit);
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
						m.hopIndex++;
						m.hops.get(0).acceptMessage(m);
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

}
