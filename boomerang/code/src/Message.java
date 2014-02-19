import java.util.ArrayList;


public class Message implements Runnable
{
	public ArrayList<Node> hops;
	public ArrayList<Long> sendTime;
	public ArrayList<Long> arriveTime;
	public String id;
	
	public Message(String id)
	{
		this.id = id;
		this.hops = new ArrayList<Node>();
	}
	
	public void setHops(ArrayList<Node> hops)
	{
		arriveTime = new ArrayList<Long>(hops.size());
		sendTime = new ArrayList<Long>(hops.size());
		for (Node n : hops)
		{
			this.hops.add(n);
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
			Thread.sleep((long)(distance));
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
