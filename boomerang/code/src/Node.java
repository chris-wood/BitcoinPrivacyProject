import java.util.ArrayList;
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
					System.out.println(id + " generating...");
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
