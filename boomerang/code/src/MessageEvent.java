
public class MessageEvent
{
	public long time;
	public int mid;
	public int srcNode;
	public int dstNode;
	
	public MessageEvent(long time, int mid, int srcNode, int dstNode)
	{
		this.time = time;
		this.mid = mid;
		this.srcNode = srcNode;
		this.dstNode = dstNode;
	}
}
