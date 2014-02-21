import java.util.ArrayList;

public class Message 
{
	public ArrayList<Node> hops;
	public ArrayList<Long> sendTime;
	public ArrayList<Long> arriveTime;
	public long broadcastTime;
	public String id;
	public MessageType type;
	public boolean startTransmit;
	public long waitTime;
	public boolean broadcasted;
	
	// State
	public Node nextHop;
	
	public Message(String id, MessageType type)
	{
		this.id = id;
		this.broadcasted = false;
		this.waitTime = 0;
		this.broadcastTime = -1;
		this.startTransmit = false;
		this.hops = new ArrayList<Node>();
	}
	
	public void doEvent()
	{
		if (waitTime > 0)
		{
			waitTime--;
		}
		else if (startTransmit) // wait time = 0
		{
			Node source = hops.remove(0);
			nextHop = hops.get(0);
			double distance = source.loc.distanceTo(nextHop.loc);
			waitTime = (long)distance;
		}
		else
		{
			nextHop.acceptMessage(this);
		}
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
		broadcastTime = Clock.time;
	}
	
	@Override
	public String toString()
	{
		return "M" + id;
	}
	
	public void transmitMessage()
	{
		startTransmit = true;
	}

//	@Override
//	public void run()
//	{
//		Node source = hops.remove(0);
//		Node nextHop = hops.get(0);
//		
//		// Sleep to emulate traversal over the network
//		try
//		{
//			double distance = source.loc.distanceTo(nextHop.loc);
//			
//			// TODO: need to scale the distance by a realistic (configurable?) factor
//			Thread.sleep(1); // (long)(distance)
//		}
//		catch (InterruptedException e)
//		{
//			e.printStackTrace();
//		} 
//		
//		// Insert into the next hop after "traversing the network"
//		try
//		{
//			nextHop.acceptMessage(this);
//		}
//		catch (InterruptedException e)
//		{
//			e.printStackTrace();
//		}
//	}
}
