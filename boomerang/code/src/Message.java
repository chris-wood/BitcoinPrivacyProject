import java.util.ArrayList;
import java.util.concurrent.Semaphore;


public class Message implements Runnable
{
	public ArrayList<Node> hops;
	public ArrayList<Long> sendTime;
	public ArrayList<Long> arriveTime;
	public String id;
	public MessageType type;
	public boolean broadcasted;
	
	// Completion semaphore
	public Semaphore broadcastSignal;
	
	public Message(String id, MessageType type, Semaphore sem)
	{
		this.broadcastSignal = sem;
		this.id = id;
		this.broadcasted = false;
		this.hops = new ArrayList<Node>();
	}
	
	public Message(String id, MessageType type)
	{
		this.id = id;
		this.broadcasted = false;
		this.hops = new ArrayList<Node>();
	}
	
	public void setHops(Node start, ArrayList<Node> hops)
	{
		arriveTime = new ArrayList<Long>(hops.size());
		sendTime = new ArrayList<Long>(hops.size());
		this.hops.add(0, start);
		for (Node n : hops)
		{
			this.hops.add(n);
		}
	}
	
	public void markAsBroadcasted()
	{
		broadcasted = true;
		if (broadcastSignal != null)
		{
			broadcastSignal.release();
		}
	}
	
	@Override
	public String toString()
	{
		return "M" + id;
	}

	@Override
	public void run()
	{
		Node source = hops.remove(0);
		Node nextHop = hops.get(0);
		
		// Sleep to emulate traversal over the network
		try
		{
			double distance = source.loc.distanceTo(nextHop.loc);
			
			// TODO: need to scale the distance by a realistic (configurable?) factor
			Thread.sleep(1); // (long)(distance)
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		} 
		
		// Insert into the next hop after "traversing the network"
		try
		{
			nextHop.acceptMessage(this);
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}
	}
}
