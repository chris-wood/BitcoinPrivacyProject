import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Random;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class Node
{
	// Behavior parameters
	public Boomerang boom;
	public int id;
	public Random rng;
	public Location loc;
	
	// Statistics
	public long startTime;
	public long endTime;
	public long liveTime;
	public int numRetries;
	public int numForwarded;
	public int numChaffEncodings;
	public int numTxEncodings;
	
	// Inner processes
	NodeForwarder forwarder;
	NodeCoverGenerator coverGenerator;
	NodeTxGenerator txGenerator;
	
	// Buffer things
	public BlockingQueue<Message> msgQueue;
	
	// Local message identifier
	public int msgIndex = 0;
	
	// For uniquely identifying nodes
	public static int globalNodeId;
	
	public Node(Location loc, Boomerang boomerang, long seed)
	{
		this.id = globalNodeId++;
		this.boom = boomerang;
		this.loc = loc;
		this.rng = new Random(seed);
		this.msgQueue = new LinkedBlockingQueue<Message>();
		
		this.numRetries = 0;
		this.numForwarded = 0;
		this.numChaffEncodings = 0;
		this.numTxEncodings = 0;
	}
	
	public void start()
	{
		startTime = System.currentTimeMillis();
		
		this.forwarder = new NodeForwarder(this);
		Thread t1 = new Thread(forwarder);
		t1.start();
		
		this.coverGenerator = new NodeCoverGenerator(this);
		Thread t2 = new Thread(coverGenerator);
		t2.start();
		
		this.txGenerator = new NodeTxGenerator(this);
		Thread t3 = new Thread(txGenerator);
		t3.start();
	}
	
	public void stop()
	{
		endTime = System.currentTimeMillis();
		liveTime = endTime - startTime;
		
		forwarder.kill();
		coverGenerator.kill();
		txGenerator.kill();
	}
	
	public void acceptMessage(Message m) throws InterruptedException
	{
		if (m.hops.size() > 1)
		{
			System.err.println(this + ": retrieved message - " + m);
			m.arriveTime.add(System.currentTimeMillis());
			msgQueue.put(m);
		}
		else
		{
			// reached the end of the circuit, broadcast the transaction here
			if (m.type == MessageType.TX)
			{
				m.markAsBroadcasted();
				System.err.println("Transaction broadcasted: " + m);
			}
			else
			{
				System.err.println("End of life: " + m);
			}
		}
	}
	
	@Override
	public String toString()
	{
		return "Node-" + id;
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
			System.err.println(host + ": killing forwarder");
			try
			{
				msgQueue.put(new Message("PP" + id, MessageType.POISON)); // poison pill
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
				for (int i = 0; i < boom.config.buffSize; i++)
				{
					try
					{
						Message m = msgQueue.take();
						if (m.type != MessageType.POISON)
						{
							bucket.add(m);
						}
						else
						{
							System.err.println("Killing forwarder " + id);
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
						host.numForwarded++;
					}
				}
			}
			
			System.err.println(host + ": forwarder going down.");
		}
	}
	
	class NodeCoverGenerator implements Runnable
	{
		private Node host;
		
		// Node thread control
		private volatile boolean running = true;
		
		public void kill()
		{
			System.err.println(host + ": killing chaff generator");
			running = false;
		}
		
		public NodeCoverGenerator(Node host)
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
					double sleep = rng.nextDouble() * boom.config.chaffGenRate;
					sleep = sleep < 0 ? (sleep * -1) % boom.config.chaffGenRate : sleep;
					System.err.println(host + ": chaff generator sleeping for: " + sleep);
					Thread.sleep((long)sleep * 1000);
					
					// Build the m circuits of length n each
					ArrayList<Message> messages = new ArrayList<Message>();
					for (int m = 0; m < boom.config.circuitWidth; m++)
					{
						ArrayList<Node> circuit = new ArrayList<Node>();
						HashSet<Integer> seen = new HashSet<Integer>();
						seen.add(host.id);
						for (int n = 0; n < boom.config.circuitDepth - 1; n++)
						{
							// Pick a new node not already in this circuit
							int nIndex = rng.nextInt(boom.getNumberOfNodes());
							while (seen.contains(nIndex) == true)
							{
								nIndex = rng.nextInt(boom.getNumberOfNodes());
							}
							
							Node node = boom.getNode(nIndex);
							circuit.add(node);
							seen.add(nIndex);
							host.numChaffEncodings++;
						}
						
						// New message to send
						Message newMsg = new Message(id + "-" + msgIndex++, MessageType.COVER);
						circuit.add(host);
						newMsg.setHops(host, circuit);
						messages.add(newMsg);					
					}
					
					// Blast out each message at the same time
					for (Message m : messages)
					{
//						m.hops.add(0, host);
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
			
			System.err.println(host + ": chaff generator going down.");
		}
	}
	
	class NodeTxGenerator implements Runnable
	{
		private Node host;
		
		// Node thread control
		private volatile boolean running = true;
		
		public void kill()
		{
			System.err.println(host + ": killing transaction generator");
			running = false;
		}
		
		public NodeTxGenerator(Node host)
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
					double sleep = rng.nextDouble() * boom.config.txGenRate;
					sleep = sleep < 0 ? (sleep * -1) % boom.config.txGenRate : sleep;
					Thread.sleep((long)sleep * 1000);
					
					// Continue to send out the new transaction until it is observed in the network
					boolean timeout = true;
					while (timeout)
					{
						// Build the m circuits of length n each
						Semaphore blockSem = new Semaphore(0); // binary sem
						ArrayList<Message> messages = new ArrayList<Message>();
						for (int m = 0; m < boom.config.circuitWidth; m++)
						{
							ArrayList<Node> circuit = new ArrayList<Node>();
							HashSet<Integer> seen = new HashSet<Integer>();
							for (int n = 0; n < boom.config.circuitDepth; n++)
							{
								// Pick a new node not already in this circuit
								int nIndex = rng.nextInt(boom.getNumberOfNodes());
								while (seen.contains(nIndex) == true)
								{
									nIndex = rng.nextInt(boom.getNumberOfNodes());
								}
								
								Node node = boom.getNode(nIndex);
								circuit.add(node);
								seen.add(nIndex);
								host.numTxEncodings++;
							}
							
							// New message to send
							Message newMsg = new Message(id + "-" + msgIndex++, MessageType.TX, blockSem);
							newMsg.setHops(host, circuit);
							messages.add(newMsg);
						}
						
						// Blast out each message at the same time
						for (Message m : messages)
						{
							m.sendTime.add(System.currentTimeMillis());
							Thread mThread = new Thread(m);
							mThread.run();
						}
						
						// Wait until at least one has gone through to the end of a circuit
						timeout = blockSem.tryAcquire(boom.config.retryLimit, TimeUnit.SECONDS);
						if (!timeout)
						{
							numRetries++;
							System.err.println("Reblasting transaction for " + host);
						}
						
						// Short circuit if the simulation ended while we were waiting
						if (!running)
						{
							System.err.println("Shortcircuiting, simuatlion done: " + host);
							timeout = false;
						}
					}
				}
				catch (InterruptedException e)
				{
					e.printStackTrace();
					running = false;
				}
			}
			
			System.err.println(host + ": transaction generator going down.");
		}
	}
}
