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
	public boolean inTransit = false;
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
		this.inTransit = false;
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
				waitTime = (long)((long)distance) + 50L;
			}
			else if (inTransit)
			{
				inTransit = false;
				nextHop.acceptMessage(this);
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
		inTransit = true;
		
		// Don't overwrite the time
		if (firstTransmit)
		{
			spawnTime = Clock.time;
			firstTransmit = false;
		}
	}
	
	public String toDataString()
	{
		StringBuilder builder = new StringBuilder();
		
		builder.append(this.toString());
		
		// Append circuit
		builder.append(" & ");
		for (int i = 0; i < circuit.size() - 1; i++)
		{
			builder.append(circuit.get(i).toString() + ",");
		}
		builder.append(circuit.get(circuit.size() - 1).toString());
		
		// Append start times
		builder.append(" & ");
		for (int i = 0; i < sendTime.size() - 1; i++)
		{
			builder.append(sendTime.get(i).toString() + ",");
		}
		builder.append(sendTime.get(sendTime.size() - 1).toString());
		
		// Append arrive times
		builder.append(" & ");
		for (int i = 0; i < arriveTime.size() - 1; i++)
		{
			builder.append(arriveTime.get(i).toString() + ",");
		}
		builder.append(arriveTime.get(arriveTime.size() - 1).toString());
		
		// Append transmit latency
		builder.append(" & ");
		builder.append(latency);
		
		return builder.toString();
	}
	
	@Override
	public String toString()
	{
		return "M-" + id;
	}
}
