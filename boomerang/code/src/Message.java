import java.util.ArrayList;


public class Message
{
	public ArrayList<Node> hops;
	public String id;
	
	public Message(String id)
	{
		this.id = id;
		this.hops = new ArrayList<Node>();
	}
}
