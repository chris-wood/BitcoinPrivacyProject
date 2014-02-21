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
	
	// Delay times for forwarding and generating
	public long forwardWait;
	public boolean forward;
	public long coverWait;
	public boolean coverSendStart;
	public long txWait;
	public boolean txSendStart;
	public boolean resend;
	
	// Inner processes
//	NodeForwarder forwarder;
//	NodeCoverGenerator coverGenerator;
//	NodeTxGenerator txGenerator;
	
	// State
	public ArrayList<Message> mixBucket;
	public ArrayList<Message> lastSent;
	public long txTimeoutWait;
	public ArrayList<Message> messages;
	
	// Buffer things
//	public BlockingQueue<Message> msgQueue;
	public ArrayList<Message> msgQueue;
	
	// Local message identifier
	public int coverMsgIndex = 0;
	public int txMsgIndex = 0;
	
	// For uniquely identifying nodes
	public static int globalNodeId;
	
	public Node(Location loc, Boomerang boomerang, long seed)
	{
		this.id = globalNodeId++;
		this.boom = boomerang;
		this.loc = loc;
		this.rng = new Random(seed);
		this.msgQueue = new ArrayList<Message>();
		this.mixBucket = new ArrayList<Message>();
		this.lastSent = new ArrayList<Message>();
		
		this.numRetries = 0;
		this.numForwarded = 0;
		this.numChaffEncodings = 0;
		this.numTxEncodings = 0;
		
		this.forwardWait = 0L;
		this.forward = false;
		this.coverWait = 0L;
		this.coverSendStart = true;
		this.txWait = 0L;
		this.txSendStart = true;
		this.resend = false;
	}
	
//	public void start()
//	{
//		startTime = System.currentTimeMillis();
//		
//		this.forwarder = new NodeForwarder(this);
//		Thread t1 = new Thread(forwarder);
//		t1.start();
//		
//		this.coverGenerator = new NodeCoverGenerator(this);
//		Thread t2 = new Thread(coverGenerator);
//		t2.start();
//		
//		this.txGenerator = new NodeTxGenerator(this);
//		Thread t3 = new Thread(txGenerator);
//		t3.start();
//	}
	
	public void doEvent()
	{
		// Handle one message in each time interval
		// TODO: should we flush all messages or just one?
		if (msgQueue.size() > 0)
		{
			Message m = msgQueue.get(0);
			msgQueue.remove(0);
			mixBucket.add(m);
		}
		if (mixBucket.size() >= boom.config.buffSize)
		{
			forward = true;
		}
		
		// Handle forwarding
		if (forward)
		{
			Collections.shuffle(mixBucket);
			for (Message m : mixBucket)
			{
				boom.numMessages.incrementAndGet();
				m.sendTime.add(Clock.time);
				m.transmitMessage();
			}
		}
		
		// Handle cover generation
		if (coverWait > 0)
		{
			coverWait--;
		} 
		else if (coverSendStart)
		{
			double sleep = rng.nextDouble() * boom.config.chaffGenRate;
			sleep = sleep < 0 ? (sleep * -1) % boom.config.chaffGenRate : sleep;
			coverWait = (long)sleep * 1000;
			coverSendStart = false;
		}
		else // generate cover
		{
			// Build the m circuits of length n each
			ArrayList<Message> messages = new ArrayList<Message>();
			for (int m = 0; m < boom.config.circuitWidth; m++)
			{
				ArrayList<Node> circuit = new ArrayList<Node>();
				HashSet<Integer> seen = new HashSet<Integer>();
				seen.add(id);
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
					numChaffEncodings++;
				}
				
				// New message to send
				Message newMsg = new Message(id + "-" + coverMsgIndex++, MessageType.COVER);
				circuit.add(this);
				newMsg.setHops(this, circuit);
				messages.add(newMsg);					
				boom.addMessage(newMsg);
			}
			
			// Blast out each message at the same time
			for (Message m : messages)
			{
				boom.numMessages.incrementAndGet();
				m.sendTime.add(Clock.time);
				m.transmitMessage();
			}
			
			coverSendStart = true;
		}
		
		// Handle TX generation
		if (txWait > 0)
		{
			txWait--;
		} 
		else if (txSendStart)
		{
			txSendStart = false;
			double sleep = rng.nextDouble() * boom.config.txGenRate;
			sleep = sleep < 0 ? (sleep * -1) % boom.config.txGenRate : sleep;
			txWait = (long)sleep * 1000;
		}
		else if (txTimeoutWait > 0)
		{	
			for (Message m : lastSent)
			{
				// So long as one instance was broadcasted, move on to the next message 
				if (m.broadcasted)
				{
					txTimeoutWait = 0;
					txSendStart = true;
				}
			}
			
			txTimeoutWait--;
			if (txSendStart = false && txTimeoutWait == 0)
			{
				resend = true;
			}
		}
		else if (resend)
		{
			// Blast out each message at the same time
			messages = new ArrayList<Message>();
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
					numTxEncodings++;
					txMsgIndex++;
				}
				
				// New message to send
				Message newMsg = new Message(id + "-" + txMsgIndex, MessageType.TX);
				newMsg.setHops(this, circuit);
				messages.add(newMsg);
				boom.addMessage(newMsg);
			}
			
			// Blast out each message at the same time
			for (Message m : messages)
			{
				boom.numMessages.incrementAndGet();
				m.sendTime.add(Clock.time);
				m.transmitMessage();
			}
			
			// Set timeout wait 
			lastSent = messages;
			txTimeoutWait = boom.config.retryLimit;
		}
		else // generate FRESH TX
		{
			messages = new ArrayList<Message>();
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
					numTxEncodings++;
				}
				
				// New message to send
				Message newMsg = new Message(id + "-" + txMsgIndex, MessageType.TX);
				newMsg.setHops(this, circuit);
				messages.add(newMsg);
				boom.addMessage(newMsg);
			}
			
			// Blast out each message at the same time
			for (Message m : messages)
			{
				boom.numMessages.incrementAndGet();
				m.sendTime.add(Clock.time);
				m.transmitMessage();
			}
			
			// Set timeout wait 
			lastSent = messages;
			txTimeoutWait = boom.config.retryLimit;
		}
	}
	
//	public void stop()
//	{
//		endTime = System.currentTimeMillis();
//		liveTime = endTime - startTime;
//		
//		forwarder.kill();
//		coverGenerator.kill();
//		txGenerator.kill();
//	}
	
	public void acceptMessage(Message m) 
	{
		if (m.hops.size() > 1)
		{
			System.err.println(this + ": retrieved message - " + m);
//			m.arriveTime.add(System.currentTimeMillis());
			m.arriveTime.add(Clock.time);
//			msgQueue.put(m);
			msgQueue.add(m);
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
	
//	class NodeForwarder implements Runnable
//	{
//		public Node host; 
//		
//		// Node thread control
//		private volatile boolean running = true;
//		
//		public NodeForwarder(Node host)
//		{
//			this.host = host;
//		}
//		
//		public void kill()
//		{
//			System.err.println(host + ": killing forwarder");
//			try
//			{
//				msgQueue.put(new Message("PP" + id, MessageType.POISON)); // poison pill
//			}
//			catch (InterruptedException e)
//			{
//				e.printStackTrace();
//			}
//			running = false;
//		}
//
//		@Override
//		public void run() 
//		{
//			while (running)
//			{
//				ArrayList<Message> bucket = new ArrayList<Message>();
//				for (int i = 0; i < boom.config.buffSize; i++)
//				{
//					try
//					{
//						Message m = msgQueue.take();
//						if (m.type != MessageType.POISON)
//						{
//							bucket.add(m);
//						}
//						else
//						{
//							System.err.println("Killing forwarder " + id);
//							running = false; // redundant
//							break;
//						}
//					}
//					catch (InterruptedException e)
//					{
//						e.printStackTrace();
//						System.exit(-1);
//					}
//				}
//				
//				if (running)
//				{
//					Collections.shuffle(bucket);
//					for (Message m : bucket)
//					{
//						boom.numMessages.incrementAndGet();
//						m.sendTime.add(System.currentTimeMillis());
//						Thread mThread = new Thread(m);
//						mThread.run();
//						host.numForwarded++;
//					}
//				}
//			}
//			
//			System.err.println(host + ": forwarder going down.");
//		}
//	}
//	
//	class NodeCoverGenerator implements Runnable
//	{
//		private Node host;
//		
//		// Node thread control
//		private volatile boolean running = true;
//		
//		public void kill()
//		{
//			System.err.println(host + ": killing chaff generator");
//			running = false;
//		}
//		
//		public NodeCoverGenerator(Node host)
//		{
//			this.host = host;
//		}
//		
//		@Override
//		public void run()
//		{
//			while (running)
//			{		
//				try
//				{
//					double sleep = rng.nextDouble() * boom.config.chaffGenRate;
//					sleep = sleep < 0 ? (sleep * -1) % boom.config.chaffGenRate : sleep;
//					System.err.println(host + ": chaff generator sleeping for: " + sleep);
//					Thread.sleep((long)sleep * 1000);
//					
//					// Build the m circuits of length n each
//					ArrayList<Message> messages = new ArrayList<Message>();
//					for (int m = 0; m < boom.config.circuitWidth; m++)
//					{
//						ArrayList<Node> circuit = new ArrayList<Node>();
//						HashSet<Integer> seen = new HashSet<Integer>();
//						seen.add(host.id);
//						for (int n = 0; n < boom.config.circuitDepth - 1; n++)
//						{
//							// Pick a new node not already in this circuit
//							int nIndex = rng.nextInt(boom.getNumberOfNodes());
//							while (seen.contains(nIndex) == true)
//							{
//								nIndex = rng.nextInt(boom.getNumberOfNodes());
//							}
//							
//							Node node = boom.getNode(nIndex);
//							circuit.add(node);
//							seen.add(nIndex);
//							host.numChaffEncodings++;
//						}
//						
//						// New message to send
//						Message newMsg = new Message(id + "-" + msgIndex++, MessageType.COVER);
//						circuit.add(host);
//						newMsg.setHops(host, circuit);
//						messages.add(newMsg);					
//					}
//					
//					// Blast out each message at the same time
//					for (Message m : messages)
//					{
//						boom.numMessages.incrementAndGet();
//						m.sendTime.add(System.currentTimeMillis());
//						Thread mThread = new Thread(m);
//						mThread.run();
//					}
//				}
//				catch (InterruptedException e)
//				{
//					e.printStackTrace();
//					running = false;
//				}
//			}
//			
//			System.err.println(host + ": chaff generator going down.");
//		}
//	}
//	
//	class NodeTxGenerator implements Runnable
//	{
//		private Node host;
//		
//		// Node thread control
//		private volatile boolean running = true;
//		
//		public void kill()
//		{
//			System.err.println(host + ": killing transaction generator");
//			running = false;
//		}
//		
//		public NodeTxGenerator(Node host)
//		{
//			this.host = host;
//		}
//		
//		@Override
//		public void run()
//		{
//			while (running)
//			{		
//				try
//				{
//					double sleep = rng.nextDouble() * boom.config.txGenRate;
//					sleep = sleep < 0 ? (sleep * -1) % boom.config.txGenRate : sleep;
//					Thread.sleep((long)sleep * 1000);
//					
//					// Continue to send out the new transaction until it is observed in the network
//					boolean timeout = true;
//					while (timeout)
//					{
//						// Build the m circuits of length n each
//						Semaphore blockSem = new Semaphore(0); // binary sem
//						ArrayList<Message> messages = new ArrayList<Message>();
//						for (int m = 0; m < boom.config.circuitWidth; m++)
//						{
//							ArrayList<Node> circuit = new ArrayList<Node>();
//							HashSet<Integer> seen = new HashSet<Integer>();
//							for (int n = 0; n < boom.config.circuitDepth; n++)
//							{
//								// Pick a new node not already in this circuit
//								int nIndex = rng.nextInt(boom.getNumberOfNodes());
//								while (seen.contains(nIndex) == true)
//								{
//									nIndex = rng.nextInt(boom.getNumberOfNodes());
//								}
//								
//								Node node = boom.getNode(nIndex);
//								circuit.add(node);
//								seen.add(nIndex);
//								host.numTxEncodings++;
//							}
//							
//							// New message to send
//							Message newMsg = new Message(id + "-" + msgIndex++, MessageType.TX, blockSem);
//							newMsg.setHops(host, circuit);
//							messages.add(newMsg);
//						}
//						
//						// Blast out each message at the same time
//						for (Message m : messages)
//						{
//							boom.numMessages.incrementAndGet();
//							m.sendTime.add(System.currentTimeMillis());
//							Thread mThread = new Thread(m);
//							mThread.run();
//						}
//						
//						// Wait until at least one has gone through to the end of a circuit
//						timeout = blockSem.tryAcquire(boom.config.retryLimit, TimeUnit.SECONDS);
//						if (!timeout)
//						{
//							numRetries++;
//							System.err.println("Reblasting transaction for " + host);
//						}
//						
//						// Short circuit if the simulation ended while we were waiting
//						if (!running)
//						{
//							System.err.println("Shortcircuiting, simuatlion done: " + host);
//							timeout = false;
//						}
//					}
//				}
//				catch (InterruptedException e)
//				{
//					e.printStackTrace();
//					running = false;
//				}
//			}
//			
//			System.err.println(host + ": transaction generator going down.");
//		}
//	}
}
