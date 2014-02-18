import java.util.ArrayList;
import java.util.Random;


public class Node implements Runnable
{
	// Behavior parameters
	public Boomerang boom;
	public int id;
	public Random rng;
	
	// Node thread control
	private volatile boolean running = true;
	
	public Node(int id, Boomerang boomerang)
	{
		this.id = id;
		this.boom = boomerang;
		this.rng = new Random(System.currentTimeMillis());
	}
	
	public void kill()
	{
		running = false;
	}
	
	@Override
	public void run()
	{
		while (running)
		{
			// do things... and then sleep
			
			try
			{
				int sleep = rng.nextInt() % boom.trafficGenRate;
				sleep = sleep < 0 ? (sleep * -1) % boom.trafficGenRate : sleep;
				Thread.sleep(sleep * 1000);
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
				running = false;
			}
		}
	}

}
