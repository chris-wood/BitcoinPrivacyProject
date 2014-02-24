import java.util.ArrayList;

public class Message 
{
	public ArrayList<Node> circuit;
	public ArrayList<Node> hops;
	public ArrayList<Long> sendTime;
	public ArrayList<Long> arriveTime;
	public long broadcastTime;
	public long spawnTime;
	public String id;
	public MessageType type;
	public boolean startTransmit;
	public long waitTime;
	public boolean broadcasted = false;
	public Boomerang boom;
	
	// Stats
	public boolean firstTransmit = true;
	public long latency;
	
	// State
	public Node nextHop;
	
	public Message(String id, Boomerang boom, MessageType type)
	{
		this.type = type;
		this.id = id;
		this.boom = boom;
		this.broadcasted = false;
		this.waitTime = 0;
		this.broadcastTime = -1;
		this.startTransmit = false;
		this.firstTransmit = true;
		this.hops = new ArrayList<Node>();
	}
	
	public void doEvent()
	{
		if (broadcasted)
		{
			boom.finalizeMessage(this);
		}
		else
		{
			if (waitTime > 0)
			{
				waitTime--;
			}
			else if (startTransmit) // wait time = 0
			{
				// Short circuit
				if (hops.size() <= 1)
				{
					markAsBroadcasted();
					return;
				}
				
				startTransmit = false;
				Node source = hops.remove(0);
				nextHop = hops.get(0);
				double distance = source.loc.distanceTo(nextHop.loc);
				broadcasted = false;
				waitTime = (long)distance;
			}
			else
			{
				nextHop.acceptMessage(this);
//				startTransmit = true;
			}
		}
	}
	
	public void setHops(Node start, ArrayList<Node> hops)
	{
		arriveTime = new ArrayList<Long>(hops.size());
		sendTime = new ArrayList<Long>(hops.size());
		this.hops.add(0, start);
		this.circuit = new ArrayList<Node>();
		for (Node n : hops)
		{
			this.hops.add(n);
			this.circuit.add(n);
		}
	}
	
	public void markAsBroadcasted()
	{
		Util.disp("Marking " + this + " as broadcasted");
		broadcasted = true;
		broadcastTime = Clock.time;
		latency = broadcastTime - sendTime.get(0);
	}
	
	public void transmitMessage()
	{
		startTransmit = true;
		
		// Don't overwrite the time
		if (firstTransmit)
		{
			spawnTime = Clock.time;
			firstTransmit = false;
		}
	}
	
	@Override
	public String toString()
	{
		return "M" + id;
	}
}
