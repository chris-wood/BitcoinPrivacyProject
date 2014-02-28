import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;


public class AddressBook
{
	public static final double VALID_THRESHOLD = 0.5;
	public static final int VALID_TIMEOUT = 180;
	
	public ArrayList<Node> nodes;
	public HashSet<Integer> invalidNodes;
	public HashMap<Node, Integer> stampMap;
	
	public AddressBook()
	{
		this.nodes = new ArrayList<Node>();
		this.invalidNodes = new HashSet<Integer>();
		this.stampMap = new HashMap<Node, Integer>();
	}
	
	public int getNumberOfValidNodes()
	{
		return nodes.size() - invalidNodes.size();
	}
	
	public int getNumberOfInvalidNodes()
	{
		return invalidNodes.size();
	}
	
	public void updateValidNodes(HashSet<Node> nodes)
	{
		invalidNodes.clear();
		for (Node n : nodes)
		{
			this.nodes.add(n);
			this.stampMap.put(n, VALID_TIMEOUT);
		}
	}
	
	public void update()
	{
		for (Node n : stampMap.keySet())
		{
			stampMap.put(n,  stampMap.get(n) - 1);
			if (stampMap.get(n) <= 0)
			{
				invalidNodes.add(n.id);
			}
		}
	}
	
	public void addValidNode(Node n)
	{
		nodes.add(n);
		stampMap.put(n, VALID_TIMEOUT);
	}
	
	public boolean isValid(int n)
	{
		return invalidNodes.contains(n);
	}
	
	public boolean isValid(Node n)
	{
		if (stampMap.containsKey(n))
		{
			return stampMap.get(n) > 0;
		}
		return false;
	}
}
