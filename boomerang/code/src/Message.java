import java.util.ArrayList;


public class Message
{
	public ArrayList<Node> hops;
	public int hopIndex;
	public String id;
	
	public Message(String id)
	{
		this.id = id;
		this.hops = new ArrayList<Node>();
	}
	
	public void setHops(ArrayList<Node> hops)
	{
		hopIndex = 0;
		for (Node n : hops)
		{
			this.hops.add(n);
		}
	}
}
